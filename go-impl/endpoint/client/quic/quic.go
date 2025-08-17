package main

// This script uses quic-go's HTTP/3 client to fetch URLs.
// It also includes a mechanism to count and log the bytes received per read operation
// and a cumulative total.

import (
	"context"
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"sync"

	"github.com/quic-go/quic-go"
	"github.com/quic-go/quic-go/http3"
	"github.com/quic-go/quic-go/qlog"
)

func main() {
	quiet := flag.Bool("q", true, "don't print the data")
	keyLogFile := flag.String("keylog", "", "key log file")
	insecure := flag.Bool("insecure", false, "skip certificate verification")
	concurrent := flag.Bool("concurrent", false, "this is the concurrent client")
	flag.Parse()
	urls := flag.Args()

	log.Printf("urls: %v", urls)

	port := "8000"
	if *concurrent {
		port = "8001"
	}

	var keyLog io.Writer
	if len(*keyLogFile) > 0 {
		f, err := os.Create(*keyLogFile)
		if err != nil {
			log.Fatal(err)
		}
		log.Println("key log file: ", *keyLogFile)
		defer f.Close()
		keyLog = f
	}

	roundTripper := &http3.Transport{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: *insecure,
			KeyLogWriter:       keyLog,
		},
		QUICConfig: &quic.Config{
			Tracer: qlog.DefaultConnectionTracer,
		},
		Dial: func(ctx context.Context, addr string, tlsCfg *tls.Config, cfg *quic.Config) (*quic.Conn, error) {
			localAddr, err := net.ResolveUDPAddr("udp", "0.0.0.0:"+port)
			if err != nil {
				panic("error after resolving udp address")
			}

			udpConn, err := net.ListenUDP("udp", localAddr)
			if err != nil {
				return nil, fmt.Errorf("error binding local UDP port: %w", err)
			}

			remoteAddr, err := net.ResolveUDPAddr("udp", addr)
			if err != nil {
				return nil, fmt.Errorf("error resolving remote UDP address: %w", err)
			}
			return quic.DialEarly(ctx, udpConn, remoteAddr, tlsCfg, cfg)
		},
	}
	defer roundTripper.Close()
	hclient := &http.Client{
		Transport: roundTripper,
	}

	var wg sync.WaitGroup
	wg.Add(len(urls))
	for _, addr := range urls {
		log.Printf("GET %s", addr)
		go func(addr string) {
			defer wg.Done()

			rsp, err := hclient.Get(addr)
			if err != nil {
				log.Fatal(err)
			}
			log.Printf("Got response for %s: %#v", addr, rsp)

			var totalBytesReceived int64
			buffer := make([]byte, 1024)
			for {
				n, err := rsp.Body.Read(buffer)
				totalBytesReceived += int64(n)
				if err == io.EOF {
					break
				}
				if err != nil {
					log.Println("Error while reading the response body")
					log.Fatal(err)
				}
			}
			if *quiet {
				log.Printf("Response Body: %d bytes", totalBytesReceived)
			} else {
				log.Printf("Response Body (%d bytes)", totalBytesReceived)
			}
		}(addr)
	}
	wg.Wait()
	log.Println("quic client is done!")
}
