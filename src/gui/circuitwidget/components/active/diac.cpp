/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include <QPainter>

#include <math.h>

#include "diac.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "e-diode.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"

Component* Diac::construct( QObject* parent, QString type, QString id )
{ return new Diac( parent, type, id ); }

LibraryItem* Diac::libraryItem()
{
    return new LibraryItem(
        tr( "Diac" ),
        tr( "Active" ),
        "diac.png",
        "Diac",
        Diac::construct );
}

Diac::Diac( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area =  QRectF( -8, -16, 16, 32 );
    setLabelPos(-12,-30, 0 );

    m_resOn    = 500;
    m_resOff   = 1e8;
    m_brkVolt  = 30;
    m_holdCurr = 0.01;

    m_pin.resize( 2 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);

    m_diode1 = new eDiode( id+"-dio1" );
    m_diode1->setEpin( 1, m_pin[1] );
    m_diode1->createSerRes();
    m_diode1->setModel( "Diode Default" );

    m_diode2 = new eDiode( id+"-dio2" );
    m_diode2->setEpin( 1, m_pin[0] );
    m_diode2->createSerRes();
    m_diode2->setModel( "Diode Default" );

    addPropGroup( { tr("Main"), {
new DoubProp<Diac>( "ResOn"   , tr("On Resistance")    ,"Ω", this, &Diac::resOn ,   &Diac::setResOn ),
new DoubProp<Diac>( "ResOff"  , tr("Off Resistance")   ,"Ω", this, &Diac::resOff,   &Diac::setResOff ),
new DoubProp<Diac>( "BrkVolt" , tr("Breakdown Voltage"),"V", this, &Diac::brkVolt,  &Diac::setBrkVolt ),
new DoubProp<Diac>( "HoldCurr", tr("Hold Current")     ,"A", this, &Diac::holdCurr, &Diac::setHoldCurr )
    }} );
}
Diac::~Diac()
{
    delete m_diode1;
    delete m_diode2;
}

void Diac::updateStep()
{
    if( Circuit::self()->animate() ) update();
}

void Diac::attach()
{
    m_state = false;

    eNode* node0 = m_pin[0]->getEnode();
    eNode* node1 = m_pin[1]->getEnode();

    m_diode1->getEpin(0)->setEnode( node0 );
    m_diode2->getEpin(0)->setEnode( node1 );
}

void Diac::stamp()
{
    eNode* node0 = m_pin[0]->getEnode();
    if( node0 ) node0->addToNoLinList( this );

    eNode* node1 = m_pin[1]->getEnode();
    if( node1 ) node1->addToNoLinList( this );

    m_diode1->setRes( m_resOff );
    m_diode2->setRes( m_resOff );
}

void Diac::voltChanged()
{
    double voltage = m_pin[0]->getVolt()-m_pin[1]->getVolt();
    double current = m_diode1->getCurrent() - m_diode2->getCurrent();
    bool state = m_state;

    if( abs(current) < m_holdCurr ) state = false;
    if( abs(voltage) > m_brkVolt  ) state = true;

    if( m_state != state )
    {
        m_state = state;
        double res = state ? m_resOn : m_resOff;
        m_diode1->setRes( res );
        m_diode2->setRes( res );
    }
}

void Diac::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points1[3] = {
        QPointF( 7,-8  ),
        QPointF(-8,-15 ),
        QPointF(-8, 0  )          };
    p->drawPolygon( points1, 3 );

 static const QPointF points2[3] = {
        QPointF(-7, 8  ),
        QPointF( 8, 0 ),
        QPointF( 8, 15 )          };
    p->drawPolygon( points2, 3 );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-8,-16,-8, 16 );
    p->drawLine( 8,-16, 8, 16 );
}
