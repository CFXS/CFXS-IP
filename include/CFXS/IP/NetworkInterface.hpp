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
#pragma once

#include <CFXS/Base/Network/IPv4.hpp>
#include <CFXS/Base/Network/MAC_Address.hpp>
#include <CFXS/IP/_Config.hpp>

namespace CFXS::IP {

    class NetworkInterface {
    public:
        NetworkInterface(const MAC_t& mac, const IP_t& ip, const Subnet_t& subnet, const IP_t& gateway = IP_t::BROADCAST());

        inline void SetMAC(const MAC_t& mac);
        inline MAC_t& GetMAC();
        inline const MAC_t& GetMAC() const;

        inline void SetIP(const IP_t& mac);
        inline IP_t& GetIP();
        inline const IP_t& GetIP() const;

        inline void SetSubnet(const Subnet_t& mac);
        inline Subnet_t& GetSubnet();
        inline const Subnet_t& GetSubnet() const;

    private:
        // Do not allow to delete
        ~NetworkInterface();

        void ConfigUpdated();

    private:
        MAC_t m_MAC;

        IP_t m_IP;
        Subnet_t m_Subnet;
        IP_t m_Gateway;
    };

} // namespace CFXS::IP
