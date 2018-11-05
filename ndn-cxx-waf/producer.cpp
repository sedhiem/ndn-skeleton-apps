#include <Consumer-Producer-API/producer-context.hpp>
#include <iostream>
#include <fstream>
#include <tuple>
#include <set>

using namespace ndn;

class CallbackContainer
{
public:
  CallbackContainer()
  : m_interestCounter(0)
  , m_dataCounter(0)
  , m_finalBlockId(0)
  {}

  void
  processInfoInterest(Producer& infoProducer, const Interest& interest)
  {
    Name filename("test.png");
    const uint8_t* m_buffer;
    size_t m_bufferSize;

    std::tie(m_buffer, m_bufferSize) = loadFile("test.png");
    int finalBlockId = infoProducer.getFinalBlockIdFromBufferSize(m_contentPrefix.append(filename), m_bufferSize);
    m_finalBlockId = (uint64_t) finalBlockId;

    std::string str(std::to_string(finalBlockId));
    std::cout << "No of Seg.: " << str << std::endl;
    infoProducer.produce(filename, reinterpret_cast<const uint8_t*>(str.c_str()), str.size());

    return;
  }

  void
  leavingInfoData(Producer& pilotProducer, const Data& data)
  {
    std::cout << "Leaving Data: " << data.getName() << std::endl;
  }

  void
  processContentInterest(Producer& contentProducer, const Interest& interest)
  {
      uint64_t segment = interest.getName().get(-1).toSegment();
      m_segmentBuffer.insert(segment);

      if(m_segmentBuffer.size() == m_finalBlockId + 1)
      {
        const uint8_t* m_buffer;
        size_t m_bufferSize;
        std::tie(m_buffer, m_bufferSize) = loadFile("test.png");
        contentProducer.produce(Name("test.png"), m_buffer, m_bufferSize);
        std::cout << "bufferSize: " << m_bufferSize << std::endl;

        std::cout << "SENT PNG FILE" << std::endl;
      }
      return;
  }

  void
  leavingContentData(Producer& producer, const Data& data)
  {
    std::cout << "Leaving Data: " << data.getName().get(-1).toSegment() << std::endl;
  }

  Name m_contentPrefix;

private:
  int m_interestCounter;
  int m_dataCounter;
  std::set<uint64_t> m_segmentBuffer;
  uint64_t m_finalBlockId;

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
    CallbackContainer callback;

    Name pilotProducerName("/test/producer/info");
    Producer pilotProducer(pilotProducerName);
    pilotProducer.setContextOption(CACHE_MISS, (ProducerInterestCallback)bind(&CallbackContainer::processInfoInterest, &callback, _1, _2));
    pilotProducer.setContextOption(DATA_LEAVE_CNTX, (ProducerDataCallback)bind(&CallbackContainer::leavingInfoData, &callback, _1, _2));
    pilotProducer.attach();

    Name contentProducerName("/test/producer/content");
    callback.m_contentPrefix = contentProducerName;
    Producer contentProducer(contentProducerName);
    contentProducer.setContextOption(FUNCTION, Name("/A"));
    contentProducer.setContextOption(CACHE_MISS, (ProducerInterestCallback)bind(&CallbackContainer::processContentInterest, &callback, _1, _2));
    contentProducer.setContextOption(DATA_LEAVE_CNTX, (ProducerDataCallback)bind(&CallbackContainer::leavingContentData, &callback, _1, _2));
    contentProducer.attach();
    sleep(300);

    return 0;
}
