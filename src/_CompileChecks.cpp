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

#if !defined(CFXS_IP_IPv4) && !defined(CFXS_IP_IPv6)
    #error not defined
#else
    #if !(defined(CFXS_IP_IPv4) ^ defined(CFXS_IP_IPv6))
        #error Multiple CFXS_IP_IPv? defined at the same time
    #endif
#endif

#if !defined(CFXS_IP_HW_PHY_INTERNAL) && !defined(CFXS_IP_HW_PHY_EXTERNAL_MII) && !defined(CFXS_IP_HW_PHY_EXTERNAL_RMII)
    #error CFXS_IP_HW_PHY_? not defined
#else
    #if !(defined(CFXS_IP_HW_PHY_INTERNAL) ^ defined(CFXS_IP_HW_PHY_EXTERNAL_MII) ^ defined(CFXS_IP_HW_PHY_EXTERNAL_RMII))
        #error Multiple CFXS_IP_HW_PHY_? defined at the same time
    #endif
#endif

#if !defined(CFXS_PLATFORM_TIVA)
    #error CFXS_PLATFORM_? not defined
#else
// multiple platform check
#endif