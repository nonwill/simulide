/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "module.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "tunnel.h"

#include "doubleprop.h"

#define tr(str) simulideTr("ModuleSubc",str)

ModuleSubc::ModuleSubc( QString type, QString id )
          : ShieldSubc( type, id )
{
    m_subcType = Chip::Module;
    setZValue( 2 );

    addPropGroup( { tr("Main"), {
new DoubProp<ModuleSubc>( "Z_Value", tr("Z Value"),"", this, &ModuleSubc::zVal, &ModuleSubc::setZVal )
    },0} );
}
ModuleSubc::~ModuleSubc(){}

void ModuleSubc::setZVal( double v )
{
    if     ( v < 2 ) v = 2;
    else if( v > 9 ) v = 9;
    setZValue( v );
}

void ModuleSubc::attachToBoard()
{
    BoardSubc* board = m_board;
    if( board->subcType() > Board )
    {
        ShieldSubc* shield = static_cast<ShieldSubc*>(board);
        BoardSubc* parentBoard = shield->parentBoard();
        if( parentBoard ) board = parentBoard;
    }
    m_boardPos = m_circPos - board->pos();
    if( board != m_board ) m_boardPos -= m_board->pos();
}

void ModuleSubc::renameTunnels()
{
    for( Tunnel* tunnel : getPinTunnels() ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}
