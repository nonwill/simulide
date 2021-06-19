/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "iocomponent.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"

IoComponent::IoComponent( QObject* parent, QString type, QString id)
           : Component( parent, type, id )
{
    m_inHighV = 2.5;
    m_inLowV  = 2.5;
    m_ouHighV   = 5;
    m_ouLowV    = 0;
    m_outValue   = 0;

    m_inImp = 1e9;
    m_ouImp   = 40;

    m_rndPD = false;
    m_invInputs = false;
    m_invOutputs  = false;

    m_propDelay = 10*1000; // 10 ns
    m_timeLH = 3000;
    m_timeHL = 4000;
}
IoComponent::~IoComponent(){}

QList<propGroup_t> IoComponent::propGroups()
{
    propGroup_t elecGroup { tr("Electric") };
    elecGroup.propList.append( {"", tr("Inputs:"),""} );
    elecGroup.propList.append( {"Input_High_V", tr("Low to High Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Low_V", tr("High to Low Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Imped", tr("Input Impedance"),"Ω"} );
    elecGroup.propList.append( {"", tr("Outputs:"),""} );
    elecGroup.propList.append( {"Out_High_V", tr("Output High Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Low_V", tr("Output Low Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Imped", tr("Output Impedance"),"Ω"} );

    propGroup_t edgeGroup { tr("Edges") };
    edgeGroup.propList.append( {"Tpd_ps", tr("Propagation Delay"),"ps"} );
    edgeGroup.propList.append( {"Tr_ps", tr("Rise Time"),"ps"} );
    edgeGroup.propList.append( {"Tf_ps", tr("Fall Time"),"ps"} );

    return {elecGroup, edgeGroup};
}

void IoComponent::updateStep()
{
    if( !Circuit::self()->animate( ) ) return;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->updateStep();
    for( uint i=0; i<m_inPin.size(); ++i )  m_inPin[i]->updateStep();
}

void IoComponent::initState()
{
    Simulator::self()->addToUpdateList( this );

    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutState( false );

    //m_outStep = 0;
    m_outValue = 0;
    m_nextOutVal = 0;
}

void IoComponent::runOutputs()
{
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        bool state = m_nextOutVal & (1<<i);
        bool oldst = m_outPin[i]->getOutState();// m_outValue   & (1<<i);

        if( state != oldst )
        {
            /*if( m_outStep == 0 )
            {
                eNode* enode =  m_output[i]->getPin()->getEnode();
                if( enode ) enode->saveData();
            }
            else */
                m_outPin[i]->setOutState( state, true );
        }
    }
    /*if( m_outStep == 0 )
    {
        m_outStep = 1;
        Simulator::self()->addEvent( m_timeLH*1.25, this );
    }
    else*/
    {
        //m_outStep = 0;
        m_outValue = m_nextOutVal;
    }
}

void IoComponent::sheduleOutPuts(  eElement* el )
{
    if( m_nextOutVal == m_outValue ) return;

    if( m_rndPD )Simulator::self()->addEvent( m_propDelay+(std::rand()%2), el );
    else         Simulator::self()->addEvent( m_propDelay, el );
}

void IoComponent::setInputHighV( double volt )
{
    if( m_inHighV == volt ) return;
    Simulator::self()->pauseSim();

    m_inHighV = volt;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputHighV( volt );

    Simulator::self()->resumeSim();
}

void IoComponent::setInputLowV( double volt )
{
    if( m_inLowV == volt ) return;
    Simulator::self()->pauseSim();

    m_inLowV = volt;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputLowV( volt );

    Simulator::self()->resumeSim();
}

void IoComponent::setOutHighV( double volt )
{
    if( m_ouHighV == volt ) return;
    Simulator::self()->pauseSim();

    m_ouHighV = volt;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutHighV( volt );

    Simulator::self()->resumeSim();
}

void IoComponent::setOutLowV( double volt )
{
    if( m_ouLowV == volt ) return;
    Simulator::self()->pauseSim();

    m_ouLowV = volt;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutLowV( volt );

    Simulator::self()->resumeSim();
}

void IoComponent::setInputImp( double imp )
{
    if( m_inImp == imp ) return;
    Simulator::self()->pauseSim();

    m_inImp = imp;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputImp( imp );

    Simulator::self()->resumeSim();
}

void IoComponent::setOutImp( double imp )
{
    if( m_ouImp == imp ) return;
    Simulator::self()->pauseSim();

    m_ouImp = imp;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutputImp( imp );

    Simulator::self()->resumeSim();
}

void IoComponent::setInvertOuts( bool inverted )
{
    if( m_invOutputs == inverted ) return;
    Simulator::self()->pauseSim();

    m_invOutputs = inverted;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setInverted( inverted );

    Circuit::self()->update();
    Simulator::self()->resumeSim();
}

void IoComponent::setInvertInps( bool invert )
{
    if( m_invInputs == invert ) return;
    Simulator::self()->pauseSim();

    m_invInputs = invert;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInverted( invert );

    Circuit::self()->update();
    Simulator::self()->resumeSim();
}

void IoComponent::setOpenCol( bool op )
{
    if( m_openCol == op ) return;
    m_openCol = op;

    for( uint i=0; i<m_outPin.size(); ++i )
    {
        if( op ) m_outPin[i]->setPinMode( open_col );
        else     m_outPin[i]->setPinMode( output );
    }
}

void IoComponent::init( QStringList pins )
{
    m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, m_height*8 );

    QStringList inputs;                                    // Input Pins
    QStringList outputs;                                  // Output Pins

    // Example: pin = "IL02Name" => input, left, number 2, label = "Name"

    for( QString pin : pins )
    {
             if( pin.startsWith( "I" ) ) inputs.append( pin );
        else if( pin.startsWith( "O" ) ) outputs.append( pin );
        else qDebug() << " LogicComponent::init: pin name error "<<pin;
    }
    int i = m_inPin.size();
    m_inPin.resize( i+inputs.length() );
    for( QString inp : inputs ) // Example input = "L02Name"
    {
        m_inPin[i] = createPin( inp, m_id+"-in"+QString::number(i) );
        i++;
    }
    i = m_outPin.size();
    m_outPin.resize( i+outputs.length() );
    for( QString out : outputs ) // Example output = "L02Name"
    {
        m_outPin[i] = createPin( out, m_id+"-out"+QString::number(i) );
        i++;
    }
}

IoPin* IoComponent::createPin( QString data, QString id )
{
    // Example data = "L02" => left side, number 2

    pinMode_t mode = (data.left(1) == "I") ? input : output ;
    data.remove(0,1);
    QString pos = data.left(1);
    data.remove(0,1);
    int num = data.left(2).toInt();
    data.remove(0,2);
    QString label = data;

    int angle = 0;
    int x = 0;
    int y = 0;

    if( pos == "U" )        // Up
    {
        angle = 90;
        x = m_area.x() + num*8;
        y = m_area.y() - 8;
    }
    else if( pos == "L")    // Left
    {
        angle = 180;
        x = m_area.x() - 8;
        y = m_area.y() + num*8;
    }
    if( pos == "D" )        // Down
    {
        angle = 270;
        x = m_area.x() + num*8;
        y = m_area.y() + m_height*8 + 8;
    }
    else if( pos == "R")    // Right
    {
        x = m_area.x() + m_width*8+8;
        y = m_area.y() + num*8;
    }
    IoPin* pin = new IoPin( angle, QPoint( x, y ), id, 0, this, mode );
    pin->setLabelColor( QColor( 0, 0, 0 ) );
    pin->setLabelText( label );
    initPin( pin );
    return pin;
}

void IoComponent::initPin( IoPin* pin )
{
    pin->setInputHighV( m_inHighV );
    pin->setInputLowV( m_inLowV );
    pin->setInputImp( m_inImp );
    pin->setOutHighV( m_ouHighV );
    pin->setOutLowV( m_ouLowV );
    pin->setOutputImp( m_ouImp  );
}

void IoComponent::setNumInps( uint pins, QString label, int bit0, bool number )
{
    setNumPins( &m_inPin, pins, label, bit0, false, number );
}

void IoComponent::setNumOuts( uint pins, QString label, int bit0, bool number )
{
    setNumPins( &m_outPin, pins, label, bit0, true, number );
}

void IoComponent::setNumPins( std::vector<IoPin*>* pinList, uint pins
                              , QString label, int bit0, bool out, bool number )
{
    uint oldSize = pinList->size();
    if( pins == oldSize ) return;
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    int halfW = (m_width/2)*8;//m_width*8/2;
    int x           = out ? halfW+8 : -(halfW)-8;
    int angle       = out ?  0  : 180;
    QString preLab  = out ? ""  : " ";
    QString PostLab = out ? " " : "";
    QString id      = out ? "-out" : "-in";
    pinMode_t mode  = out ? output : input;

    id = m_id+id;

    if( pins < oldSize ) deletePins( pinList, oldSize-pins );
    else                 pinList->resize( pins );

    if( m_outPin.size() > m_inPin.size() ) m_height = m_outPin.size();
    else                                   m_height = m_inPin.size();

    int halfH;
    if( label.isEmpty() ) halfH = m_height*8/2; // Gates
    else
    {
        m_height += 1;
        halfH = (m_height/2)*8;
    }

    m_area = QRect(-halfW,-halfH, m_width*8, m_height*8 );

    int start = 8;
    if( label.isEmpty() ) start = 4;  // Gates
    else if( start%8 ) start +=4;

    for( uint i=0; i<pins; ++i )
    {
        int y = m_area.y() + i*8 + start;

        QString num = "";
        QString pinId = id;
        if( i < oldSize ) pinList->at(i)->setY( y );
        else
        {
            if( number )
            {
                pinId += QString::number(i);
                num = QString::number(i+bit0);
            }
            pinList->at(i) = new IoPin( angle, QPoint( x, y ), pinId, i, this, mode );
            initPin( pinList->at(i) );

            if( !label.isEmpty() ) pinList->at(i)->setLabelText( preLab+label+num+PostLab );
            pinList->at(i)->setLabelColor( QColor( 0, 0, 0 ) );
        }
    }
    Circuit::self()->update();
}

void IoComponent::deletePins( std::vector<IoPin*>* pinList, uint pins )
{
    uint oldSize = pinList->size();
    if( pins > oldSize ) pins = oldSize;

    uint newSize = oldSize-pins;

    for( uint i=oldSize-1; i>newSize-1; --i )
    {
        pinList->at(i)->removeConnector();
        if( pinList->at(i)->scene() ) Circuit::self()->removeItem( pinList->at(i) );
        pinList->at(i)->reset();
        delete pinList->at(i);
    }
    pinList->resize( newSize );
}

void IoComponent::remove()
{
    Simulator::self()->remFromUpdateList( this );

    for( uint i=0; i<m_inPin.size(); i++ )  m_inPin[i]->removeConnector();
    for( uint i=0; i<m_outPin.size(); i++ ) m_outPin[i]->removeConnector();

    Component::remove();
}
void IoComponent::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );
}

#include "moc_iocomponent.cpp"