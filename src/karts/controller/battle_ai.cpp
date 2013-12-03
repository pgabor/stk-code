//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2012 Joerg Henrichs
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


#include "karts/controller/battle_ai.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/skidding.hpp"
#include "karts/skidding_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "tracks/nav_poly.hpp"
#include "tracks/navmesh.hpp"

#include <iostream>

BattleAI::BattleAI(AbstractKart *kart, 
                    StateManager::ActivePlayer *player) 
                       : AIBaseController(kart, player)
{

    reset();
    
    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES)
    {
        m_world     = dynamic_cast<ThreeStrikesBattle*>(World::getWorld());
        m_track     = m_world->getTrack();
        
    }
    else
    {
        // Those variables are not defined in a battle mode (m_world is
        // a linear world, since it assumes the existance of drivelines)
        m_world           = NULL;
        m_track           = NULL;
    }

    updateCurrentNode(); 

    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kar.
    Controller::setControllerName("BattleAI");

}

void BattleAI::update(float dt)
{
    m_controls->m_accel     = 0.65f;
 //   m_controls->m_steer     = 0;
    updateCurrentNode();
 
    handleSteering(dt);
}


void BattleAI::reset()
{
    m_current_node = BattleGraph::UNKNOWN_POLY;
}

void BattleAI::updateCurrentNode()
{
    //std::cout<<"Current Node \t"<< m_current_node << std::endl;

    // if unknown location, search everywhere
    if(m_current_node == BattleGraph::UNKNOWN_POLY)
    {
        int max_count = BattleGraph::get()->getNumNodes();
        for(unsigned int i =0; i<max_count; i++)
        {
            const NavPoly& p = BattleGraph::get()->getPolyOfNode(i);
            if(p.pointInPoly(m_kart->getXYZ()))
                m_current_node = i;
        }
        return;
    }

    if(m_current_node != BattleGraph::UNKNOWN_POLY)
    {
        //check if the kart is still on the same node
        const NavPoly& p = BattleGraph::get()->getPolyOfNode(m_current_node);
        if(p.pointInPoly(m_kart->getXYZ())) return;

        //if not then check all adjacent polys
        const std::vector<int>& adjacents = 
                            NavMesh::get()->getAdjacentPolys(m_current_node);
        
        // Set m_current_node to unknown so that if no adjacent poly checks true
        // we look everywhere the next time updateCurrentNode is called. This is
        // useful in cases when you are "teleported" to some other poly, ex. rescue
        m_current_node = BattleGraph::UNKNOWN_POLY;
        
        
        for(unsigned int i=0; i<adjacents.size(); i++)
        {
            const NavPoly& p_temp = 
                    BattleGraph::get()->getPolyOfNode(adjacents[i]);
            if(p_temp.pointInPoly(m_kart->getXYZ())) m_current_node = adjacents[i];
        }
        return;
    }
}


void BattleAI::handleSteering(const float dt)
{
    Vec3  target_point;
    const AbstractKart* kart = m_world->getPlayerKart(0);
    int player_node = -1;
    for(unsigned int i =0; i<BattleGraph::get()->getNumNodes(); i++)
        {
            const NavPoly& p = BattleGraph::get()->getPolyOfNode(i);
            if(p.pointInPoly(kart->getXYZ()))
                player_node = i;
        }
    
    if(player_node == BattleGraph::UNKNOWN_POLY || m_current_node == BattleGraph::UNKNOWN_POLY) return;
    int next_node = BattleGraph::get()->getNextShortestPathPoly(m_current_node, player_node);
    if(next_node == -1) return;
    target_point = NavMesh::get()->getCenterOfPoly(next_node);
    
    if(player_node != m_current_node)
    setSteering(steerToPoint(target_point),dt);
    else
        setSteering(steerToPoint(kart->getXYZ()),dt);
}