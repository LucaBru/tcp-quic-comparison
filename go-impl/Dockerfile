FROM martenseemann/quic-network-simulator-endpoint:latest

RUN wget https://dl.google.com/go/go1.24.5.linux-amd64.tar.gz && \
  tar xfz go1.24.5.linux-amd64.tar.gz && \
  rm go1.24.5.linux-amd64.tar.gz

ENV PATH="/go/bin:${PATH}"
ENV GO111MODULE=on

COPY quic-go /quic-go
WORKDIR /quic-go

RUN go build -C example/client .
RUN go build -C example/tcp .
RUN go build -C example .

COPY run_endpoint.sh .
RUN chmod +x run_endpoint.sh
COPY run_clients.sh .
RUN chmod +x run_clients.sh

ENTRYPOINT ["./run_endpoint.sh"]
