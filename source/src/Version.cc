/// \file Version.cc
/*
 *
 * Version.cc source template automatically generated by a class generator
 * Creation date : lun. juin 8 2015
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
#include <dqm4hep/Version.h>

namespace dqm4hep {

  namespace core {

    Version::Version() : m_major(0), m_minor(0), m_patch(0) {
      /* nop */
    }

    //-------------------------------------------------------------------------------------------------

    Version::Version(unsigned int major, unsigned int minor, unsigned int patch) {
      set(major, minor, patch);
    }

    //-------------------------------------------------------------------------------------------------

    Version::~Version() {
      /* nop */
    }

    //-------------------------------------------------------------------------------------------------

    unsigned int Version::getMajor() const {
      return m_major;
    }

    //-------------------------------------------------------------------------------------------------

    unsigned int Version::getMinor() const {
      return m_minor;
    }

    //-------------------------------------------------------------------------------------------------

    unsigned int Version::getPatch() const {
      return m_patch;
    }

    //-------------------------------------------------------------------------------------------------

    void Version::set(unsigned int major, unsigned int minor, unsigned int patch) {
      m_major = major;
      m_minor = minor;
      m_patch = patch;
      std::stringstream ss;
      ss << m_major << "." << m_minor << "." << m_patch;
      m_versionString = ss.str();
    }

    //-------------------------------------------------------------------------------------------------

    const std::string &Version::toString() const {
      return m_versionString;
    }

    //-------------------------------------------------------------------------------------------------

    Version &Version::operator=(const Version &version) {
      m_major = version.m_major;
      m_minor = version.m_minor;
      m_patch = version.m_patch;
      m_versionString = version.toString();

      return *this;
    }

    //-------------------------------------------------------------------------------------------------

    bool operator<(const Version &lhs, const Version &rhs) {
      if (lhs.getMajor() < rhs.getMajor())
        return true;

      if (lhs.getMinor() < rhs.getMinor())
        return true;

      if (lhs.getPatch() < rhs.getPatch())
        return true;

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    bool operator<=(const Version &lhs, const Version &rhs) {
      if (lhs < rhs)
        return true;

      if (lhs.getMajor() == rhs.getMajor() && lhs.getMinor() == rhs.getMinor() && lhs.getPatch() == rhs.getPatch())
        return true;

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    bool operator>(const Version &lhs, const Version &rhs) {
      return !(lhs <= rhs);
    }

    //-------------------------------------------------------------------------------------------------

    bool operator>=(const Version &lhs, const Version &rhs) {
      return !(lhs < rhs);
    }

    //-------------------------------------------------------------------------------------------------

    bool operator==(const Version &lhs, const Version &rhs) {
      return (lhs.getMajor() == rhs.getMajor() && lhs.getMinor() == rhs.getMinor() && lhs.getPatch() == rhs.getPatch());
    }

    //-------------------------------------------------------------------------------------------------

    bool operator!=(const Version &lhs, const Version &rhs) {
      return !(lhs == rhs);
    }

    //-------------------------------------------------------------------------------------------------

    xdrstream::Status Version::stream(xdrstream::StreamingMode mode, xdrstream::IODevice *pDevice,
                                      xdrstream::xdr_version_t /*version*/) {
      if (xdrstream::XDR_READ_STREAM == mode) {
        uint32_t major, minor, patch;
        XDR_STREAM(pDevice->read(&major));
        XDR_STREAM(pDevice->read(&minor));
        XDR_STREAM(pDevice->read(&patch));

        this->set(major, minor, patch);
      } else {
        XDR_STREAM(pDevice->write<uint32_t>(&m_major));
        XDR_STREAM(pDevice->write<uint32_t>(&m_minor));
        XDR_STREAM(pDevice->write<uint32_t>(&m_patch));
      }

      return xdrstream::XDR_SUCCESS;
    }
  }
}
