FROM alpine:3.18 AS builder

RUN apk add --no-cache build-base make openssl-dev zlib-dev brotli-dev

WORKDIR /app

COPY httplib.h /app/
COPY project/ /app/project/

WORKDIR /app/project

RUN make server OPENSSL_SUPPORT="" ZLIB_SUPPORT="-lz" BROTLI_SUPPORT=""


FROM alpine:3.18

RUN apk add --no-cache libstdc++ libgcc

WORKDIR /app

COPY --from=builder /app/project/server /app/server

ENV PORT=8080

RUN adduser -D myuser && \
    chown myuser /app/server && \
    rm -rf /var/cache/apk/*

USER myuser

EXPOSE 8080

ENTRYPOINT ["./server"]
