/// \file Archiver.cc
/*
 *
 * Archiver.cc source template automatically generated by a class generator
 * Creation date : mar. oct. 7 2014
 *
 * This file is part of DQM4HEP libraries.
 *
 * DQM4HEP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * DQM4HEP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DQM4HEP.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */

// -- dqm4hep headers
#include "dqm4hep/Archiver.h"
#include "dqm4hep/Directory.h"
#include "dqm4hep/Logging.h"
#include "dqm4hep/MonitorElement.h"
#include "dqm4hep/Storage.h"

// -- root headers
#include "TDirectory.h"
#include "TFile.h"
#include "TSystem.h"

namespace dqm4hep {

  namespace core {

    Archiver::Archiver() {
      m_selectorFunction = [](MonitorElementPtr)->bool{return true;};
    }

    //-------------------------------------------------------------------------------------------------

    Archiver::~Archiver() {
      if (isOpened())
        close();
    }

    //-------------------------------------------------------------------------------------------------

    StatusCode Archiver::open(const std::string &fname, const std::string &opMode, bool overwrite, int runNumber) {
      // if already open write the archive if not done
      // and close it before to re-open
      if (isOpened()) {
        RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, close());
      }
      if (fname.empty()) {
        return STATUS_CODE_INVALID_PARAMETER;
      }
      int fileId(0);
      size_t pos = fname.rfind(".root");
      if (std::string::npos == pos) {
        dqm_error("Couldn't open archive '{0}' ! Must be a root file !", fname);
        return STATUS_CODE_INVALID_PARAMETER;
      }
      std::string baseArchiveName = fname.substr(0, pos);
      std::string fullArchiveName = fname;
      if (not overwrite) {
        while (!gSystem->AccessPathName(fullArchiveName.c_str())) {
          std::stringstream ss;
          ss << baseArchiveName;
          if(runNumber >= 0) {
            ss << "_I" << runNumber;
          }
          ss << "_" << fileId << ".root";
          fullArchiveName = ss.str();
          fileId++;
        }
        m_fileName = fullArchiveName;
      } 
      else if(runNumber >= 0) {
        std::stringstream ss;
        ss << baseArchiveName << "_I" << runNumber << ".root";
        m_fileName = ss.str();
      }
      else {
        m_fileName = fname;
      }
      m_openingMode = opMode;
      dqm_info("Archiver::open: Opening archive {0}", m_fileName);
      m_file.reset(new TFile(m_fileName.c_str(), m_openingMode.c_str()));
      if (nullptr == m_file) {
        dqm_error("Archiver::open: Couldn't open archive '{0}' !", m_fileName);
        return STATUS_CODE_FAILURE;
      }
      m_isOpened = true;
      return STATUS_CODE_SUCCESS;
    }

    //-------------------------------------------------------------------------------------------------

    StatusCode Archiver::close() {
      if (not isOpened()) {
        return STATUS_CODE_SUCCESS;
      }
      m_file->Close();
      m_file.reset(nullptr);
      m_isOpened = false;
      m_openingMode = "";
      return STATUS_CODE_SUCCESS;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Archiver::setSelectorFunction(std::function<bool(MonitorElementPtr)> func) {
      m_selectorFunction = func;
    }

    //-------------------------------------------------------------------------------------------------

    StatusCode Archiver::archive(const Storage<MonitorElement> &storage, const std::string &dirName) {
      TDirectory *directory = nullptr;
      RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, prepareForArchiving(dirName, directory));
      RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, recursiveWrite(storage.root(), directory, ""));
      m_file->cd();
      m_file->Write();
      return STATUS_CODE_SUCCESS;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    StatusCode Archiver::archiveWithReferences(const Storage<MonitorElement> &storage, const std::string &dirName, const std::string &refSuffix) {
      TDirectory *directory = nullptr;
      RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, prepareForArchiving(dirName, directory));
      RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, recursiveWrite(storage.root(), directory, refSuffix));
      m_file->cd();
      m_file->Write();
      return STATUS_CODE_SUCCESS;
    }

    //-------------------------------------------------------------------------------------------------

    const std::string &Archiver::fileName() const {
      return m_fileName;
    }

    //-------------------------------------------------------------------------------------------------

    bool Archiver::isOpened() const {
      return m_isOpened;
    }

    //-------------------------------------------------------------------------------------------------

    const std::string &Archiver::openingMode() const {
      return m_openingMode;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    StatusCode Archiver::prepareForArchiving(const std::string &dirName, TDirectory *&directory) {
      if(not isOpened()) {
        return STATUS_CODE_NOT_INITIALIZED;
      }
      directory = nullptr;
      if (!dirName.empty()) {
        directory = m_file->mkdir(dirName.c_str());
        if (directory == nullptr) {
          return STATUS_CODE_FAILURE;
        }
      } 
      else {
        directory = m_file.get();
      }
      return STATUS_CODE_SUCCESS;
    }

    //-------------------------------------------------------------------------------------------------

    StatusCode Archiver::recursiveWrite(MonitorElementDir directory, TDirectory *rootDirectory, const std::string &refSuffix) {
      if (nullptr == directory || nullptr == rootDirectory) {
        return STATUS_CODE_INVALID_PTR;
      }
      rootDirectory->cd();
      const auto &subDirList(directory->subdirs());
      for (auto iter = subDirList.begin(), endIter = subDirList.end(); endIter != iter; ++iter) {
        auto subDir = *iter;
        TDirectory *rootSubDirectory = rootDirectory->mkdir(subDir->name().c_str());
        if (nullptr != rootSubDirectory) {
          RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, Archiver::recursiveWrite(subDir, rootSubDirectory, refSuffix));
        }
      }
      // write the monitor elements
      if (not directory->isEmpty()) {
        RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, Archiver::writeMonitorElements(directory, rootDirectory, refSuffix));
      }
      return STATUS_CODE_SUCCESS;
    }

    //-------------------------------------------------------------------------------------------------

    StatusCode Archiver::writeMonitorElements(MonitorElementDir directory, TDirectory *rootDirectory, const std::string &refSuffix) {
      if (nullptr == directory || nullptr == rootDirectory) {
        return STATUS_CODE_INVALID_PTR;
      }
      rootDirectory->cd();
      const auto &monitorElementList(directory->contents());
      for (auto iter = monitorElementList.begin(), endIter = monitorElementList.end(); endIter != iter; ++iter) {
        if(not m_selectorFunction(*iter)) {
          continue;
        }
        const std::string writeName = (*iter)->name();
        TObject *object = (*iter)->object();
        if(nullptr != object) {
          rootDirectory->WriteObjectAny(object, object->IsA(), writeName.c_str());
          TObject *reference = (*iter)->reference();
          if(not refSuffix.empty() and nullptr != reference) {
            rootDirectory->WriteObjectAny(reference, reference->IsA(), (writeName + refSuffix).c_str());
          }
        }
      }
      return STATUS_CODE_SUCCESS;
    }
    
    //-------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------

    ArchiverSelector::ArchiverSelector() {
      m_function = [this](MonitorElementPtr element)->bool{
        if(m_selectorFunctions.empty()) {
          return true;
        }
        for(auto &selector : m_selectorFunctions) {
          if(selector(element)) {
            dqm_debug( "Archiving element path: {0}, name: {1} ...", element->path(), element->name() );
            return true;
          }
        }
        dqm_debug( "Skipping element path: {0}, name: {1} !", element->path(), element->name() );
        return false;
      };
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void ArchiverSelector::addSelector(Archiver::SelectorFunction selector) {
      m_selectorFunctions.push_back(selector);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    const Archiver::SelectorFunction &ArchiverSelector::function() const {
      return m_function;
    }
    
  }

}
