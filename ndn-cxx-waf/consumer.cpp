#include<Consumer-Producer-API/consumer-context.hpp>
#include<iostream>
#include<fstream>

using namespace ndn;

class CallbackContainer
{
public:
  CallbackContainer()
  {
  }

  void
  processPayload(Consumer& consumer, const uint8_t* buffer, size_t bufferSize)
  {
    std::cout << "bufferSize: " << bufferSize << std::endl;
    std::cout << "Creating File" << std::endl;
    createFilefromBuffer(buffer, bufferSize);
    std::cout << "DONE" << std::endl;

    return;
  }

  void
  createFilefromBuffer(const uint8_t* buffer, size_t bufferSize)
  {
    std::ofstream outfile("test2.png", std::ofstream::binary);
    outfile.write((const char*)buffer, bufferSize);
    outfile.close();

    return;
  }
};

int main(int argc, char* argv[])
{
  Name consumerName("/test/producer");

  CallbackContainer callback;

  Consumer consumer(consumerName, RDR);
  consumer.setContextOption(MUST_BE_FRESH_S, true);
  consumer.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processPayload, &callback, _1, _2, _3));
  consumer.consume("test.png");

  return 0;
}
