//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifdef ENABLE_WIIUSE

#include "wiimote_manager.hpp"
#include "wiiuse.h"
#include "graphics/irr_driver.hpp"

WiimoteManager*  wiimote_manager;

static const int   NB_MAX_WIIMOTES = 2;

WiimoteManager::WiimoteManager()
{
    m_wiimotes = NULL;
    m_nb_wiimotes = 0;
}

WiimoteManager::~WiimoteManager()
{
    cleanup();
}

void WiimoteManager::launchDetection(int timeout)
{
    cleanup();
    
    m_wiimotes =  wiiuse_init(NB_MAX_WIIMOTES);
    
    // Detect wiimotes
    int nb_found_wiimotes = wiiuse_find(m_wiimotes, NB_MAX_WIIMOTES, timeout);
    
    // Try to connect to all found wiimotes
    m_nb_wiimotes = wiiuse_connect(m_wiimotes, nb_found_wiimotes);
    
    // Set the LEDs and rumble for 0.2s
    int leds[] = {WIIMOTE_LED_1, WIIMOTE_LED_2, WIIMOTE_LED_3, WIIMOTE_LED_4};
    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        wiiuse_set_leds(m_wiimotes[i], leds[i]);
        wiiuse_rumble(m_wiimotes[i], 1);
    }
    
    irr_driver->getDevice()->sleep(200);

    for(int i=0 ; i < m_nb_wiimotes ; i++)
        wiiuse_rumble(m_wiimotes[i], 0);
}

// -----------------------------------------------------------------------------
void WiimoteManager::update()
{
    if(m_nb_wiimotes > 0)
    {
        if(wiiuse_poll(m_wiimotes, NB_MAX_WIIMOTES))
        {
            for (int i=0; i < NB_MAX_WIIMOTES; ++i)
            {
                switch (m_wiimotes[i]->event)
                {
                case WIIUSE_EVENT:
                    printf("DEBUG: wiimote event\n");
                    break;
    
                case WIIUSE_STATUS:
                    printf("DEBUG: status event\n");
                    break;
    
                case WIIUSE_DISCONNECT:
                case WIIUSE_UNEXPECTED_DISCONNECT:
                    printf("DEBUG: wiimote disconnected\n");
                    break;
    
                case WIIUSE_READ_DATA:
                    printf("DEBUG: WIIUSE_READ_DATA\n");
                    break;
    
                case WIIUSE_NUNCHUK_INSERTED:
                    printf("DEBUG: Nunchuk inserted.\n");
                    break;
    
                case WIIUSE_CLASSIC_CTRL_INSERTED:
                    printf("DEBUG: Classic controller inserted.\n");
                    break;
    
                case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
                    printf("DEBUG: Guitar Hero 3 controller inserted.\n");
                    break;
    
                case WIIUSE_NUNCHUK_REMOVED:
                case WIIUSE_CLASSIC_CTRL_REMOVED:
                case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
                    printf("DEBUG: An expansion was removed.\n");
                    break;
                default:
                    break;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
void WiimoteManager::cleanup()
{
    if(m_nb_wiimotes > 0)
    {
        wiiuse_cleanup(m_wiimotes, NB_MAX_WIIMOTES);
        m_wiimotes = NULL;
        m_nb_wiimotes = 0;
    }
}

#endif // ENABLE_WIIUSE