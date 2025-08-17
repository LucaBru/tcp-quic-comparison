FROM martenseemann/quic-network-simulator-endpoint:latest

RUN apt-get update && apt-get install -y git openssl

RUN openssl req -x509 -newkey rsa:4096 -nodes -keyout server.key -out server.crt -days 365 -subj "/CN=localhost"

RUN wget https://dl.google.com/go/go1.24.5.linux-amd64.tar.gz && \
  tar xfz go1.24.5.linux-amd64.tar.gz && \
  rm go1.24.5.linux-amd64.tar.gz

ENV PATH="/go/bin:${PATH}"
ENV GO111MODULE=on

COPY endpoint /endpoint
WORKDIR /endpoint

COPY run_endpoint.sh .
COPY run_clients.sh .
RUN chmod +x run_endpoint.sh
RUN chmod +x run_clients.sh

RUN go build -C client/tcp -o tcp .
RUN go build -C client/quic -o quic .
RUN go build -o server .


ENTRYPOINT ["./run_endpoint.sh"]
