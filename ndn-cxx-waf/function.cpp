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
#include <Python.h>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>


// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`

class Func
{
public:
  void
  run()
  {
    m_face.setInterestFilter(m_prefix,
                             bind(&Func::onInterest, this, _1, _2),
                             ndn::RegisterPrefixSuccessCallback(),
                             bind(&Func::onRegisterFailed, this, _1, _2));

    m_face.processEvents();
  }

  void
  setPrefix(ndn::Name prefix)
  {
    m_prefix = prefix;
  }

private:
  void
  onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest)
  {
    std::cout << "<< Interest: " << interest << std::endl;
    std::cout << "Function Success: " << interest.getFunction() << std::endl;

    interest.removeHeadFunction();
    static const std::string content = "Success";

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
    std::cout << data << std::endl;
    /*std::string content(reinterpret_cast<const char*>(data.getContent().value()), data.getContent().value_size());
    auto pos = content.find("0");
    content.replace(pos, 1, "1");
    std::cout << "<<New Content: " << content << std::endl;
    data.setContent2(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());*/

    Py_Initialize();
    PyObject* PyFileObject = PyFile_FromString("testpython.py", "r");
    PyRun_SimpleFileEx(PyFile_AsFile(PyFileObject), "testpython.py", 1);
    Py_Finalize();

    std::ifstream filetext;
    std::string line;
    std::string stringtext;
    filetext.open("testtext.txt");
    while(std::getline(filetext, line)) {
      stringtext = stringtext + line;
    }
    filetext.close();

    std::cout << stringtext << std::endl;
    std::shared_ptr<ndn::Data> newdata = std::make_shared<ndn::Data>(interest.getName());
    newdata->setContent(reinterpret_cast<const uint8_t*>(stringtext.c_str()), stringtext.size());

    m_keyChain.sign(*newdata);
    m_face.put(*newdata);
  }

  void
  onNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
  {
    std::cout << "received Nack with reason " << nack.getReason()
              << " for interest " << interest << std::endl;
  }

  void
  onTimeout(const ndn::Interest& interest)
  {
    std::cout << "Timeout " << interest << std::endl;
  }

  void
  onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_face.shutdown();
  }

private:
  ndn::Face m_face;
  ndn::Name m_prefix;
  ndn::KeyChain m_keyChain;
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
    function.setPrefix(ndn::Name(argv[1]));
    function.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
