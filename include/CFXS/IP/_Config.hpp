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

namespace CFXS {

    class MAC_Address;
    class IPv4;
    class IPv6;

    namespace IP {

        using MAC_t = MAC_Address;
#if CFXS_IP_IPv4
        using IP_t     = IPv4;
        using Subnet_t = IP_t;
#else
        using IP_Address = IPv6;
        using Subnet_t   = uint8_t;
#endif

    } // namespace IP
} // namespace CFXS