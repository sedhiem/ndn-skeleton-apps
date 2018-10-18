/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2018 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 */

// correct way to include ndn-cxx headers
// #include <ndn-cxx/face.hpp>
// #include <ndn-cxx/security/key-chain.hpp>
//#include <Python.h>
#include <Consumer-Producer-API/producer-context.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>


class Func
{
public:
  void
  run()
  {
    m_face.setInterestFilter(m_funcName,
                             bind(&Func::onInterest, this, _1, _2),
                             ndn::RegisterPrefixSuccessCallback(),
                             bind(&Func::onRegisterFailed, this, _1, _2));

    m_face.processEvents();
  }

  void
  setFuncName(ndn::Name funcName)
  {
    m_funcName = funcName;
    m_lastReassembledSegment = 0;
    m_finalBlockNumber = 0;
  }

private:
  void
  onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest)
  {
    std::cout << "<< Interest: " << interest.getName() << std::endl;
    //std::cout << "Function Success: " << interest.getFunction() << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    interest.removeHeadFunction();
    //static const std::string content = "Success";

    interest.refreshNonce();
    // Return Data packet to the requester
    m_face.expressInterest(interest,
                           bind(&Func::onData, this,  _1, _2),
                           bind(&Func::onNack, this, _1, _2),
                           bind(&Func::onTimeout, this, _1));

    //std::cout << interest.getFunction() << std::endl;

  }

  void
  onData(const ndn::Interest& interest, const ndn::Data& data)
  {
    dataCreation(data);
    //m_face.put(data);
  }

  /***********************************DATA REASSEMBLY*************************************/
  void
  dataCreation(const ndn::Data& data)
  {
    std::cout << "Interest: " << data.getName() << std::endl;
    std::cout << "Prefix: " << m_prefix << std::endl;
    std::cout << "Segment No.: " << data.getName().get(-1).toSegment() << std::endl;
    std::cout << "Final Block No.: " << m_finalBlockNumber << std::endl;

    if(data.getContentType() == ndn::tlv::ContentType_Blob)
    {
      m_receiveBuffer[data.getName().get(-1).toSegment()] = data.shared_from_this();
      reassembleSegments();


      if(data.getName().get(-1).toSegment() == 0)
      {
        m_prefix = getPrefix(data.getName());
        std::cout << "Prefix: " << m_prefix << std::endl;
        m_finalBlockNumber = data.getFinalBlockId().toSegment();
        m_face.put(data);
      }

      if(data.getName().get(-1).toSegment() == m_finalBlockNumber)
      {
        createFile();

        /**APP GOES HERE**/

        dataSegmentation(ndn::Name("test.png"));
      }
    }
    std::cout << "-------------------------------------------------------" << std::endl;
  }

  void
  reassembleSegments()
  {
    auto head = m_receiveBuffer.find(m_lastReassembledSegment);
    while(head != m_receiveBuffer.end())
    {
      addToBuffer(*(head->second));
      m_receiveBuffer.erase(head);
      m_lastReassembledSegment++;
      head = m_receiveBuffer.find(m_lastReassembledSegment);
    }
  }

  void
  addToBuffer(const ndn::Data& data)
  {
    const ndn::Block content = data.getContent();
    m_contentBuffer.insert(m_contentBuffer.end(), &content.value()[0], &content.value()[content.value_size()]);
    std::cout << "Adding to Buffer" << std::endl;

    return;
  }

  void
  createFile()
  {
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Creating File" << std::endl;
    std::ofstream outfile("test3.png", std::ofstream::binary);
    outfile.write((char*)m_contentBuffer.data(), m_contentBuffer.size());
    std::cout << "BufferSize: " << m_contentBuffer.size() << std::endl;
    outfile.close();
    std::cout << "Success" << std::endl;
    m_finalBlockNumber = 0;

    return;
  }

  /*********************************************************************************/

  /************************DATA SEGMENTATION****************************************/
  void
  dataSegmentation(ndn::Name suffix)
  {
    const uint8_t* buffer;
    size_t bufferSize;
    std::tie(buffer, bufferSize) = loadFile("test3.png");

    ndn::Producer producer(m_prefix);
    producer.attach();
    producer.produce(suffix, buffer, bufferSize);
    sleep(300);
  }

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
  /*********************************************************************************/

  /************************Callbacks************************************************/
  void
  onNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
  {
    std::cout << "received Nack with reason " << nack.getReason()
              << " for interest " << interest << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
  }

  void
  onTimeout(const ndn::Interest& interest)
  {
    std::cout << "Timeout Seg No.: " << interest.getName().get(-1).toSegment() << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
  }

  void
  onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_face.shutdown();
  }
/***************************************************************************************/

  ndn::Name
  getPrefix(ndn::Name name){
    std::string prefix;
    for(int i = 0; i < name.size()-2; i++) //Removes Segment and file name
    {
      prefix.append("/");
      prefix.append(name.get(i).toUri());
    }
    return ndn::Name(prefix);
  }

private:
  ndn::Face m_face;
  ndn::Name m_funcName;
  ndn::KeyChain m_keyChain;

  ndn::Name m_prefix;
  std::map<uint64_t, std::shared_ptr<const ndn::Data>> m_receiveBuffer;
  std::vector<uint8_t> m_contentBuffer;
  uint64_t m_lastReassembledSegment;
  uint64_t m_finalBlockNumber;
};

int
main(int argc, char** argv)
{
  if(argc < 2){
    std::cerr << "Input Prefix for Function" << std::endl;
    return 1;
  }
  try {
    Func function;
    function.setFuncName(ndn::Name(argv[1]));
    function.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
