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
#ifdef CFXS_PLATFORM_TIVA

    #include <CFXS/Base/Debug.hpp>
    #include <CFXS/IP/_LoggerConfig.hpp>
    #include <CFXS/IP/NetworkInterface.hpp>
    #include <CFXS/Base/CPU.hpp>
    #include <CFXS/Base/Platform.hpp>
    #include <CFXS/Base/MAC_Address.hpp>
    #include <driverlib/sysctl.h>
    #include <driverlib/emac.h>
    #include <driverlib/rom.h>
    #include <driverlib/interrupt.h>
    #include <inc/hw_memmap.h>
    #include <inc/hw_emac.h>
    #include <inc/hw_ints.h>

    #include <cstdlib> // malloc

    #if defined(CFXS_IP_HW_PHY_INTERNAL)
        #define EMAC_PHY_CONFIG EMAC_PHY_TYPE_INTERNAL
        #define PHY_PHYS_ADDR   0
    #elif defined(CFXS_IP_HW_PHY_EXTERNAL_MII)
        #define EMAC_PHY_CONFIG EMAC_PHY_TYPE_EXTERNAL_MII
        #define PHY_PHYS_ADDR   1
    #elif defined(CFXS_IP_HW_PHY_EXTERNAL_RMII)
        #define EMAC_PHY_CONFIG EMAC_PHY_TYPE_EXTERNAL_RMII
        #define PHY_PHYS_ADDR   1
    #endif

namespace CFXS::IP {

    // psNetif->output     = etharp_output;
    // psNetif->linkoutput = tivaif_transmit;

    struct Descriptor {
        tEMACDMADescriptor desc;
        uint8_t buffer[2048];
    };

    struct DescriptorList {
        Descriptor *descriptors;
        uint32_t descriptorCount;
        uint32_t read;
        uint32_t write;
    };

    Descriptor s_TX_Descriptors[CFXS_IP_HW_TX_DESCRIPTORS];
    Descriptor s_RX_Descriptors[CFXS_IP_HW_RX_DESCRIPTORS];
    DescriptorList s_TX_DescriptorList = {s_TX_Descriptors, CFXS_IP_HW_TX_DESCRIPTORS, 0, 0};
    DescriptorList s_RX_DescriptorList = {s_RX_Descriptors, CFXS_IP_HW_RX_DESCRIPTORS, 0, 0};

    static void InitializeDescriptors() {
        for (int i = 0; i < CFXS_IP_HW_TX_DESCRIPTORS; i++) {
            //s_TX_Descriptors[i].buffer         = nullptr;
            s_TX_Descriptors[i].desc.ui32Count = 0;
            s_TX_Descriptors[i].desc.pvBuffer1 = 0;
            s_TX_Descriptors[i].desc.DES3.pLink =
                ((i == (CFXS_IP_HW_TX_DESCRIPTORS - 1)) ? &s_TX_Descriptors[0].desc : &s_TX_Descriptors[i + 1].desc);
            s_TX_Descriptors[i].desc.ui32CtrlStatus = DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_IP_ALL_CKHSUMS;
        }

        s_TX_DescriptorList.read  = 0;
        s_TX_DescriptorList.write = 0;

        for (int i = 0; i < CFXS_IP_HW_RX_DESCRIPTORS; i++) {
            //s_RX_Descriptors[i].buffer         = (uint8_t *)malloc(2048);
            memset(s_RX_Descriptors[i].buffer, 0, 2048);
            s_RX_Descriptors[i].desc.ui32Count = DES1_RX_CTRL_CHAINED;
            if (s_RX_Descriptors[i].buffer) {
                /* Set the DMA to write directly into the pbuf payload. */
                //s_RX_Descriptors[i].desc.pvBuffer1 = s_RX_Descriptors[i].pBuf->payload;
                //s_RX_Descriptors[i].desc.ui32Count |= (s_RX_Descriptors[i].pBuf->len << DES1_RX_CTRL_BUFF1_SIZE_S);
                s_RX_Descriptors[i].desc.pvBuffer1 = s_RX_Descriptors[i].buffer;
                s_RX_Descriptors[i].desc.ui32Count |= (2048 << DES1_RX_CTRL_BUFF1_SIZE_S);
                s_RX_Descriptors[i].desc.ui32CtrlStatus = DES0_RX_CTRL_OWN;
            } else {
                NetworkLogger_Base::LogWarning("Buffer not available for RX descriptor %u", i);
                s_RX_Descriptors[i].desc.pvBuffer1      = 0;
                s_RX_Descriptors[i].desc.ui32CtrlStatus = 0;
            }
            s_RX_Descriptors[i].desc.DES3.pLink =
                ((i == (CFXS_IP_HW_RX_DESCRIPTORS - 1)) ? &s_RX_Descriptors[0].desc : &s_RX_Descriptors[i + 1].desc);
        }

        s_TX_DescriptorList.read  = 0;
        s_TX_DescriptorList.write = 0;

        EMACRxDMADescriptorListSet(EMAC0_BASE, &s_RX_Descriptors[0].desc);
        EMACTxDMADescriptorListSet(EMAC0_BASE, &s_TX_Descriptors[0].desc);
    }

    bool InitializeEthernetHardware(NetworkInterface *netif) {
        char tempStr[32];
        NetworkLogger_Base::Log("Initialize ethernet hardware");

        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
        ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);

    #ifdef CFXS_IP_HW_PHY_INTERNAL
        if (ROM_SysCtlPeripheralPresent(SYSCTL_PERIPH_EPHY0)) {
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
            ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
            NetworkLogger_Base::Log(" - Internal PHY enabled");
        } else {
            NetworkLogger_Base::LogError(" - Initialization failed: internal PHY not present");
            CFXS_BKPT();
            return false;
        }
    #endif

        while (!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0)) {}

    #ifdef PART_TM4C1292NCPDT
        NetworkLogger_Base::Log(" - Configure pins for external PHY");

        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
        ROM_GPIOPinConfigure(GPIO_PP0_EN0INTRN);
        GPIOPinTypeEthernetMII(GPIO_PORTP_BASE, GPIO_PIN_0);
        ROM_GPIOPinConfigure(GPIO_PK4_EN0RXD3);
        GPIOPinTypeEthernetMII(GPIO_PORTK_BASE, GPIO_PIN_4);
        ROM_GPIOPinConfigure(GPIO_PK6_EN0TXD2);
        GPIOPinTypeEthernetMII(GPIO_PORTK_BASE, GPIO_PIN_6);
        ROM_GPIOPinConfigure(GPIO_PG5_EN0TXD1);
        GPIOPinTypeEthernetMII(GPIO_PORTG_BASE, GPIO_PIN_5);
        ROM_GPIOPinConfigure(GPIO_PA6_EN0RXCK);
        GPIOPinTypeEthernetMII(GPIO_PORTA_BASE, GPIO_PIN_6);
        ROM_GPIOPinConfigure(GPIO_PG7_EN0RXDV);
        GPIOPinTypeEthernetMII(GPIO_PORTG_BASE, GPIO_PIN_7);
        ROM_GPIOPinConfigure(GPIO_PG3_EN0TXEN);
        GPIOPinTypeEthernetMII(GPIO_PORTG_BASE, GPIO_PIN_3);
        ROM_GPIOPinConfigure(GPIO_PQ5_EN0RXD0);
        GPIOPinTypeEthernetMII(GPIO_PORTQ_BASE, GPIO_PIN_5);
        ROM_GPIOPinConfigure(GPIO_PM6_EN0CRS);
        GPIOPinTypeEthernetMII(GPIO_PORTM_BASE, GPIO_PIN_6);
        ROM_GPIOPinConfigure(GPIO_PK5_EN0RXD2);
        GPIOPinTypeEthernetMII(GPIO_PORTK_BASE, GPIO_PIN_5);
        ROM_GPIOPinConfigure(GPIO_PG4_EN0TXD0);
        GPIOPinTypeEthernetMII(GPIO_PORTG_BASE, GPIO_PIN_4);
        ROM_GPIOPinConfigure(GPIO_PG2_EN0TXCK);
        GPIOPinTypeEthernetMII(GPIO_PORTG_BASE, GPIO_PIN_2);
        ROM_GPIOPinConfigure(GPIO_PQ6_EN0RXD1);
        GPIOPinTypeEthernetMII(GPIO_PORTQ_BASE, GPIO_PIN_6);
        ROM_GPIOPinConfigure(GPIO_PK7_EN0TXD3);
        GPIOPinTypeEthernetMII(GPIO_PORTK_BASE, GPIO_PIN_7);
        ROM_GPIOPinConfigure(GPIO_PM7_EN0COL);
        GPIOPinTypeEthernetMII(GPIO_PORTM_BASE, GPIO_PIN_7);
    #endif

        EMACPHYConfigSet(EMAC0_BASE, EMAC_PHY_CONFIG);

        ROM_EMACInit(EMAC0_BASE, CFXS::CPU::CLOCK_FREQUENCY, EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED, 4, 4, 0);

        ROM_EMACConfigSet(
            EMAC0_BASE,
            (EMAC_CONFIG_FULL_DUPLEX | EMAC_CONFIG_CHECKSUM_OFFLOAD | EMAC_CONFIG_7BYTE_PREAMBLE | EMAC_CONFIG_IF_GAP_96BITS |
             EMAC_CONFIG_USE_MACADDR0 | EMAC_CONFIG_SA_FROM_DESCRIPTOR | EMAC_CONFIG_BO_LIMIT_1024),
            (EMAC_MODE_RX_STORE_FORWARD | EMAC_MODE_TX_STORE_FORWARD | EMAC_MODE_TX_THRESHOLD_64_BYTES | EMAC_MODE_RX_THRESHOLD_64_BYTES),
            0);

        ROM_EMACAddrSet(EMAC0_BASE, 0, netif->GetMAC().GetDataPointer());

        netif->SetMTU(1500);

        InitializeDescriptors();

    #if defined(CFXS_IP_HW_PHY_EXTERNAL_MII) || defined(CFXS_IP_HW_PHY_EXTERNAL_RMII)
        EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, EPHY_BMCR_MIIRESET);
        while ((EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR) & EPHY_BMCR_MIIRESET) == EPHY_BMCR_MIIRESET)
            ;
    #endif

        uint16_t temp;

        // Clear any stray PHY interrupts that may be set
        temp = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1);
        temp = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR2);

        // Configure and enable the link status change interrupt in the PHY
        temp = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_SCR);
        temp |= (EPHY_SCR_INTEN_EXT | EPHY_SCR_INTOE_EXT);
        EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_SCR, temp);
        EMACPHYWrite(
            EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1, (EPHY_MISR1_LINKSTATEN | EPHY_MISR1_SPEEDEN | EPHY_MISR1_DUPLEXMEN | EPHY_MISR1_ANCEN));

        // Read the PHY interrupt status to clear any stray events
        temp = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1);

        EMACFrameFilterSet(EMAC0_BASE, (EMAC_FRMFILTER_HASH_AND_PERFECT | EMAC_FRMFILTER_PASS_MULTICAST));

        // PTP
        // Enable timestamping on all received packets.
        //
        // We set the fine clock adjustment mode and configure the subsecond
        // increment to half the 25MHz PTPD clock.  This will give us maximum control
        // over the clock rate adjustment and keep the arithmetic easy later.  It
        // should be possible to synchronize with higher accuracy than this with
        // appropriate juggling of the subsecond increment count and the addend
        // register value, though.
        //
        //EMACTimestampConfigSet(EMAC0_BASE,
        //                       (EMAC_TS_ALL_RX_FRAMES | EMAC_TS_DIGITAL_ROLLOVER | EMAC_TS_PROCESS_IPV4_UDP | EMAC_TS_ALL |
        //                        EMAC_TS_PTP_VERSION_1 | EMAC_TS_UPDATE_FINE),
        //                       (1000000000 / (25000000 / 2)));
        //EMACTimestampAddendSet(EMAC0_BASE, 0x80000000);
        //EMACTimestampEnable(EMAC0_BASE);

        EMACIntClear(EMAC0_BASE, EMACIntStatus(EMAC0_BASE, false));

        EMACTxEnable(EMAC0_BASE);
        EMACRxEnable(EMAC0_BASE);

        EMACIntEnable(
            EMAC0_BASE,
            (EMAC_INT_RECEIVE | EMAC_INT_TRANSMIT | EMAC_INT_TX_STOPPED | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_PHY));

        IntEnable(INT_EMAC0);

        // Start auto negotiation
    #if defined(CFXS_IP_HW_PHY_EXTERNAL_MII) || defined(CFXS_IP_HW_PHY_EXTERNAL_RMII)
        EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, (EPHY_BMCR_SPEED | EPHY_BMCR_DUPLEXM | EPHY_BMCR_ANEN | EPHY_BMCR_RESTARTAN));
    #else
        EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, (EPHY_BMCR_ANEN | EPHY_BMCR_RESTARTAN));
    #endif

        NetworkLogger_Base::Log(" - Initialized with MAC: %s", netif->GetMAC().PrintTo(tempStr, sizeof(tempStr)));
        return true;
    }

} // namespace CFXS::IP

#endif