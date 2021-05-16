/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "mcupin.h"
#include "mcuport.h"
#include "simulator.h"

McuPin::McuPin( McuPort* port, int i, QString id, Component* mcu )
      : IoPin( 0, QPoint(0,0), mcu->objectName()+id, 0, mcu, source )
{
    m_port   = port;
    m_number = i;
    m_id     = id;

    m_pinMask = 1<<i;

    m_outState = false;
    m_openColl = false;
    m_puMask   = false;
    m_dirMask  = false;

    //Pin* pin = new Pin( 0, QPoint( 0, 0 ), mcu->objectName()+id, i, mcu );
    //m_ePin[0] = pin;

    setOutHighV( 5 );
    setPinMode( input );
    initialize();
}
McuPin::~McuPin() {}

void McuPin::initialize()
{
    m_extCtrl = false;
    m_inpState = false;

    setDirection( m_dirMask );
    setPullup( m_puMask );

    IoPin::initialize();
}

void McuPin::stamp()
{
    if( m_ePin[0]->isConnected() ) // Outputs are also called so they set Input register if needed
        m_ePin[0]->getEnode()->voltChangedCallback( this ); // Receive voltage change notifications

    IoPin::stamp();
}

void McuPin::voltChanged()
{
    bool state = m_ePin[0]->getVolt() > digital_thre;

    if( state == m_inpState ) return;
    m_inpState = state;

    m_port->pinChanged( m_pinMask, state );
}

void McuPin::setPortState( bool state )
{
    m_outState = state;
    if( !m_isOut ) return;
    IoPin::setOutState( state, true );
}

void McuPin::setOutState( bool state, bool )
{
    if( m_extCtrl ) IoPin::setOutState( state, true );
}

bool McuPin::getInpState()
{
    return m_inpState;
}

void McuPin::setDirection( bool out )
{
    m_isOut = out;

    if( out )       // Set Pin to Output
    {
        if( m_openColl ) m_oldPinMode =  open_col;
        else             m_oldPinMode =  output;

        IoPin::setOutState( m_outState );
    }
    else           // Set Pin to Input
    {
        m_oldPinMode = input;
    }
    if( !m_extCtrl ) setPinMode( m_oldPinMode ); // Is someone is controlling us, just save Pin Mode
}

void McuPin::setPullup( bool up )
{
    m_pullup = up;

    if( up ) m_vddAdmEx = 1/1e5; // Activate pullup
    else     m_vddAdmEx = 0;     // Deactivate pullup

    update();
}

void McuPin::setExtraSource( double vddAdmit, double gndAdmit ) // Comparator Vref out to Pin for example
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;

    update();
}

