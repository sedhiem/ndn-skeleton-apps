#include <Consumer-Producer-API/producer-context.hpp>
#include <iostream>
#include <fstream>
#include <tuple>

using namespace ndn;

class CallbackContainer
{
public:
  CallbackContainer()
  : m_interestCounter(0)
  {}

  void processInterest(Producer& producer, const Interest& interest)
  {
    m_interestCounter++;

    std::cout << "LOADING PNG" << std::endl;

    const uint8_t* buffer;
    size_t bufferSize;
    std::tie(buffer, bufferSize) = loadFile("test.png");

    std::cout << "SENDING PNG" << std::endl;

    producer.produce(Name("test.png"), buffer, bufferSize);
    std::cout << "bufferSize: " << bufferSize << std::endl;

    std::cout << "SENT PNG FILE" << std::endl;
  }

private:
  int m_interestCounter;

  std::tuple<const uint8_t*, size_t> loadFile(std::string filename)
  {
      std::ifstream infile(filename, std::ifstream::binary);
      infile.seekg(0, infile.end);
      size_t bufferSize = infile.tellg();
      infile.seekg(0);

      char* buffer = new char[bufferSize];
      infile.read(buffer, bufferSize);
      infile.close();

      return std::make_tuple((const uint8_t*)buffer, bufferSize);
  }
};

int main(int argc, char* argv[])
{
    Name producerName("/test/producer");

    CallbackContainer callback;

    Producer producer(producerName);

    producer.setContextOption(CACHE_MISS, (ProducerInterestCallback)bind(&CallbackContainer::processInterest, &callback, _1, _2));
    producer.attach();
    sleep(300);

    return 0;
}
