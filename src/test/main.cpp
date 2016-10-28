#include "binary_protocol.h"
#include "buffer_stream.h"
#include "file_stream.h"
#include "buffer.h"

using namespace krpc;

int main(int argc, char** argv)
{
    FileStream* file = new FileStream("./demo.txt", "w");
    file->open();

    Stream* stream  = new FramedStream(file);
    Protocol* protocol = new BinaryProtocol(stream);

    protocol->writeByte(1);
    protocol->writeI16(2);
    protocol->writeI32(3);
    protocol->writeI64(4);
    protocol->writeDouble(5);
    Buffer* buf = new Buffer();
    buf->assign("hello world", strlen("hello world"));
    protocol->writeBytes(buf);

    stream->flush();
    file->close();

    fprintf(stderr, "write ok\n");

    file->open("r");

    int8_t byte;
    protocol->readByte(byte);
    fprintf(stderr, "%d\n", byte);

    int16_t i16;
    protocol->readI16(i16);
    fprintf(stderr, "%d\n", i16);

    int i32;
    protocol->readI32(i32);
    fprintf(stderr, "%d\n", i32);

    int64_t i64;
    protocol->readI64(i64);
    fprintf(stderr, "%ld\n", i64);

    double d;
    protocol->readDouble(d);
    fprintf(stderr, "%f\n", d);

    protocol->readBytes(buf);
    fprintf(stderr, "%s:%d\n", buf->data(), buf->size());
    fprintf(stderr, "read ok\n");
}

