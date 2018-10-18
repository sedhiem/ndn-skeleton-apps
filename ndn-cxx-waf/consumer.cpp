#include<Consumer-Producer-API/consumer-context.hpp>
#include<iostream>
#include<fstream>

using namespace ndn;

class CallbackContainer
{
public:
  CallbackContainer()
  : m_dataCounter(0)
  {
  }

  void
  leavingInfoInterest(Consumer& pilotConsumer, Interest& interest)
  {
    std::cout << "Leaving Info: " << interest.getName() << std::endl;

    return;
  }

  void
  leavingContentInterest(Consumer& contentConsumer, Interest& interest)
  {
    //std::cout << "Leaving Content: " << interest.getName() << std::endl;

    return;
  }

  void
  processInfoData(Consumer& pilotConsumer, const Data& data)
  {
      std::string str(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
      int finalBlockId = std::stoi(str);
      std::cout << "finalBlockId: " << finalBlockId << std::endl;
      std::cout << "------------------------------------------------------" << std::endl;

      return;
  }

  void
  processContentPayload(Consumer& contentConsumer, const uint8_t* buffer, size_t bufferSize)
  {
    std::cout << "Received All Segments" << std::endl;
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
private:
  int m_dataCounter;
};

int main(int argc, char* argv[])
{
  CallbackContainer callback;

  Name pilotConsumerName("test/producer/info");
  Consumer pilotConsumer(pilotConsumerName, RDR);
  pilotConsumer.setContextOption(MUST_BE_FRESH_S, true);
  pilotConsumer.setContextOption(INTEREST_LIFETIME, 5000);
  pilotConsumer.setContextOption(DATA_ENTER_CNTX, (ConsumerDataCallback)bind(&CallbackContainer::processInfoData, &callback, _1, _2));
  pilotConsumer.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::leavingInfoInterest, &callback, _1, _2));
  pilotConsumer.consume("test.png");

  Name contentConsumerName("/test/producer/content");
  Name functionName("/A");
  Consumer contentConsumer(contentConsumerName, RDR);
  contentConsumer.setContextOption(MUST_BE_FRESH_S, true);
  contentConsumer.setContextOption(INTEREST_LIFETIME, 10000);
  contentConsumer.setContextOption(MAX_WINDOW_SIZE, 300);
  contentConsumer.setContextOption(CONTENT_RETRIEVED, (ConsumerContentCallback)bind(&CallbackContainer::processContentPayload, &callback, _1, _2, _3));
  contentConsumer.setContextOption(INTEREST_LEAVE_CNTX, (ConsumerInterestCallback)bind(&CallbackContainer::leavingContentInterest, &callback, _1, _2));
  contentConsumer.setContextOption(FUNCTION, functionName);
  contentConsumer.consume("test.png");

  return 0;
}