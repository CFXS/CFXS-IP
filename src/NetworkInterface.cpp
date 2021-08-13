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
    extern bool InitializeEthernetHardware(NetworkInterface* netif);

    static int s_NetworkInterfaceID = 0;
    //////////////////////////////////////////////////////////////

    NetworkInterface::NetworkInterface(const MAC_t& mac, const IP_t& ip, const Subnet_t& subnet, const IP_t& gateway) :
        m_ID(s_NetworkInterfaceID++), m_MAC(mac), m_IP(ip), m_Subnet(subnet), m_Gateway(gateway) {
        char tempStr[32];
        CFXS_ASSERT(s_NetworkInterfaceID > 0, "Only one network interface supported");
        CFXS_ASSERT(ip.IsValidHostAddress(), "Invalid IP: %s", ip.PrintTo(tempStr, sizeof(tempStr)));
#ifdef CFXS_IP_IPv4
        CFXS_ASSERT(subnet.IsValidSubnetMask(), "Invalid Subnet: %s", subnet.PrintTo(tempStr, sizeof(tempStr)));
#else
        CFXS_ASSERT(subnet >= 1 && subnet <= 127, "Invalid Subnet: %u", subnet);
#endif

        NetworkLogger_Base::LogConstructor("NetworkInterface[%p]()", this);
        NetworkLogger_Base::Log(" - MAC:     %s", mac.PrintTo(tempStr, sizeof(tempStr)));
        NetworkLogger_Base::Log(" - IP:      %s", ip.PrintTo(tempStr, sizeof(tempStr)));
#ifdef CFXS_IP_IPv4
        NetworkLogger_Base::Log(" - Subnet:  %s", subnet.PrintTo(tempStr, sizeof(tempStr)));
#else
        NetworkLogger_Base::Log(" - Subnet:  %u", subnet);
#endif
        NetworkLogger_Base::Log(" - Gateway: %s", gateway.PrintTo(tempStr, sizeof(tempStr)));
    }

    NetworkInterface::~NetworkInterface() {
        CFXS_ASSERT(0, "~NetworkInterface() not allowed");
    }

    //////////////////////////////////////////////////////////////

    bool NetworkInterface::InitializeHardware() {
        if (IsHardwareInitialized()) {
            NetworkLogger_Base::LogWarning("NetworkInterface[%p] hardware already initialized", this);
            return true;
        } else {
            SetHardwareInitialized(InitializeEthernetHardware(this));
            return IsHardwareInitialized();
        }
    }

    bool NetworkInterface::IsHardwareInitialized() const {
        return m_HardwareInitialized;
    }

    void NetworkInterface::SetHardwareInitialized(bool inited) {
        m_HardwareInitialized = inited;
    }

    //////////////////////////////////////////////////////////////

    void NetworkInterface::SetMTU(int mtu) {
        m_MTU = mtu;
    }

    int NetworkInterface::GetMTU() const {
        return m_MTU;
    }

    int NetworkInterface::GetID() const {
        return m_ID;
    }

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