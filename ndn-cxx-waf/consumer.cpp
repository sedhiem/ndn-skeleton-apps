#include <Consumer-Producer-API/consumer-context.hpp>
#include <fstream>
#include <iostream>

using namespace ndn;

class CallbackContainer
{
public:
  CallbackContainer() : m_dataCounter(0) {}

  void leavingInfoInterest(Consumer &pilotConsumer, Interest &interest)
  {
    std::cout << "Leaving Info: " << interest.getName() << std::endl;

    return;
  }

  void leavingContentInterest(Consumer &contentConsumer, Interest &interest)
  {
    std::cout << "Leaving Content: " << interest.getName().get(-1).toSegment()
              << " " << interest.getFunction() << std::endl;
    return;
  }

  void processInfoData(Consumer &pilotConsumer, const Data &data)
  {
    std::cout << "data: " << data.getName() << std::endl;
    std::string str(reinterpret_cast<const char *>(data.getContent().value()),
                    data.getContent().value_size());
    int finalBlockId = std::stoi(str);
    std::cout << "finalBlockId: " << finalBlockId << std::endl;
    std::cout << "------------------------------------------------------"
              << std::endl;
    m_finalBlockId = (uint64_t)finalBlockId;
    return;
  }

  void processContentData(Consumer &pilotConsumer, const Data &data)
  {
    std::cout << "------------------Received Data-------------------------"
              << std::endl;
    std::cout << "data: " << data.getName() << std::endl;
    std::cout << "------------------------------------------------------"
              << std::endl;
    return;
  }

  void processContentPayload(Consumer &contentConsumer, const uint8_t *buffer,
                             size_t bufferSize)
  {
    std::cout << "Received All Segments" << std::endl;
    std::cout << "bufferSize: " << bufferSize << std::endl;
    std::cout << "Creating File" << std::endl;
    createFilefromBuffer(buffer, bufferSize);
    std::cout << "DONE" << std::endl;

    return;
  }

  void createFilefromBuffer(const uint8_t *buffer, size_t bufferSize)
  {
    std::ofstream outfile("savedtest.jpg", std::ofstream::binary);
    outfile.write((const char *)buffer, bufferSize);
    outfile.close();

    return;
  }

  uint64_t m_finalBlockId;

private:
  int m_dataCounter;
};

int main(int argc, char *argv[])
{
  CallbackContainer callback;

  /*Name pilotConsumerName("test/producer/info");
  Consumer pilotConsumer(pilotConsumerName, SDR);
  pilotConsumer.setContextOption(MUST_BE_FRESH_S, true);
  pilotConsumer.setContextOption(INTEREST_LIFETIME, 5000);
  pilotConsumer.setContextOption(DATA_ENTER_CNTX,
  (ConsumerDataCallback)bind(&CallbackContainer::processInfoData, &callback, _1,
  _2));
  pilotConsumer.setContextOption(INTEREST_LEAVE_CNTX,
  (ConsumerInterestCallback)bind(&CallbackContainer::leavingInfoInterest,
  &callback, _1, _2));
  pilotConsumer.consume("test.jpg");*/

  Name contentConsumerName("/test/producer/content");
  Name functionName("/A");
  Consumer contentConsumer(contentConsumerName, RDR);
  contentConsumer.setContextOption(MUST_BE_FRESH_S, true);
  contentConsumer.setContextOption(INTEREST_LIFETIME, 1000000);
  contentConsumer.setContextOption(MAX_WINDOW_SIZE, 300);
  contentConsumer.setContextOption(
    CONTENT_RETRIEVED,
    (ConsumerContentCallback)bind(&CallbackContainer::processContentPayload,
                                  &callback, _1, _2, _3));
  contentConsumer.setContextOption(
    DATA_ENTER_CNTX,
    (ConsumerDataCallback)bind(&CallbackContainer::processContentData,
                               &callback, _1, _2));
  contentConsumer.setContextOption(
    INTEREST_LEAVE_CNTX,
    (ConsumerInterestCallback)bind(&CallbackContainer::leavingContentInterest,
                                   &callback, _1, _2));
  contentConsumer.setContextOption(FUNCTION, functionName);
  contentConsumer.setContextOption(EraseCache, 1);
  // contentConsumer.setContextOption(FINAL_BLOCK_ID, callback.m_finalBlockId);
  // std::cout << callback.m_finalBlockId << std::endl;
  contentConsumer.consume("test.jpg");

  return 0;
}
