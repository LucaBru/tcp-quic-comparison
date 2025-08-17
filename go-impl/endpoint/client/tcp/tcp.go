package main

import (
	"crypto/tls"
	"crypto/x509"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"sync"

	"golang.org/x/net/http2"
)

func main() {
	keyLogFile := flag.String("keylog", "", "key log file (for TLS debugging)")
	insecure := flag.Bool("insecure", false, "skip certificate verification")
	concurrent := flag.Bool("concurrent", false, "this is the concurrent client")
	output := flag.String("output", "download.dat", "output file")
	flag.Parse()
	urls := flag.Args()

	if len(urls) == 0 {
		log.Fatal("no URLs provided")
	}

	port := 8000
	if *concurrent {
		port = 8001
	}

	pool, err := x509.SystemCertPool()
	if err != nil {
		log.Fatal(err)
	}
	tlsConf := &tls.Config{
		RootCAs:            pool,
		InsecureSkipVerify: *insecure,
		NextProtos:         []string{"h2", "http/1.1"},
	}

	var keyLog io.Writer
	if *keyLogFile != "" {
		f, err := os.Create(*keyLogFile)
		if err != nil {
			log.Fatal(err)
		}
		defer f.Close()
		keyLog = f
		tlsConf.KeyLogWriter = keyLog
		log.Println("key log file: ", *keyLogFile)
	}

	dialer := &net.Dialer{
		LocalAddr: &net.TCPAddr{IP: net.IPv4zero, Port: port},
	}

	transport := &http.Transport{
		TLSClientConfig: tlsConf,
		DialContext:     dialer.DialContext,
	}

	if err := http2.ConfigureTransport(transport); err != nil {
		log.Fatal(err)
	}

	client := &http.Client{
		Transport: transport,
	}

	var wg sync.WaitGroup
	wg.Add(len(urls))

	for _, url := range urls {
		go func(url string) {
			defer wg.Done()
			var downloaded int64 = 0

			// Open file in append mode
			file, err := os.OpenFile(*output, os.O_CREATE|os.O_WRONLY, 0644)
			if err != nil {
				log.Fatal(err)
			}
			defer file.Close()

			for {
				req, err := http.NewRequest("GET", url, nil)
				if err != nil {
					log.Fatal(err)
				}

				if downloaded > 0 {
					req.Header.Set("Range", fmt.Sprintf("bytes=%d-", downloaded))
				}

				resp, err := client.Do(req)
				if err != nil {
					log.Printf("Error, reconnecting: %v", err)
					continue // retry on network error
				}

				if resp.StatusCode != http.StatusOK && resp.StatusCode != http.StatusPartialContent {
					log.Fatalf("unexpected status code: %d", resp.StatusCode)
				}

				n, err := io.Copy(file, resp.Body)
				downloaded += n
				log.Println("downloaded ", downloaded)
				resp.Body.Close()

				if err == nil || err == io.EOF {
					break
				} else {
					log.Printf("Download interrupted, will resume...")
				}
			}

			log.Printf("Download complete: %d bytes from %s", downloaded, url)
		}(url)
	}

	wg.Wait()
	log.Println("tcp client is done!")
}
