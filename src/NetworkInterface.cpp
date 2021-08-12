// ---------------------------------------------------------------------
// CFXS Framework Ethernet Module <https://github.com/CFXS/CFXS-IP>
// Copyright (C) 2021 | CFXS / Rihards Veips
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
// ---------------------------------------------------------------------
// [CFXS] //

#include <CFXS/IP/NetworkInterface.hpp>
#include <CFXS/IP/_LoggerConfig.hpp>

namespace CFXS::IP {

    //////////////////////////////////////////////////////////////

    NetworkInterface::NetworkInterface(const MAC_t& mac, const IP_t& ip, const Subnet_t& subnet, const IP_t& gateway) :
        m_MAC(mac), m_IP(ip), m_Subnet(subnet), m_Gateway(gateway) {
        NetworkLogger_Base::LogConstructor("NetworkInterface() [0x%p]\n", this);
    }

    NetworkInterface::~NetworkInterface() {
        CFXS_ASSERT(0, "~NetworkInterface() not allowed");
    }

    //////////////////////////////////////////////////////////////

    void NetworkInterface::SetMAC(const MAC_t& mac) {
        if (m_MAC != mac) {
            m_MAC = mac;
            ConfigUpdated();
        }
    }

    MAC_t& NetworkInterface::GetMAC() {
        return m_MAC;
    }

    const MAC_t& NetworkInterface::GetMAC() const {
        return m_MAC;
    }

    void NetworkInterface::SetIP(const IP_t& ip) {
        if (m_IP != ip) {
            m_IP = ip;
            ConfigUpdated();
        }
    }

    IP_t& NetworkInterface::GetIP() {
        return m_IP;
    }

    const IP_t& NetworkInterface::GetIP() const {
        return m_IP;
    }

    void NetworkInterface::SetSubnet(const Subnet_t& subnet) {
        if (m_Subnet != subnet) {
            m_Subnet = subnet;
            ConfigUpdated();
        }
    }

    Subnet_t& NetworkInterface::GetSubnet() {
        return m_Subnet;
    }

    const Subnet_t& NetworkInterface::GetSubnet() const {
        return m_Subnet;
    }

} // namespace CFXS::IP