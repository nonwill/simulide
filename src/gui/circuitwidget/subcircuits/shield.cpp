/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shield.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "tunnel.h"
#include "node.h"
#include "e-node.h"
//#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"
#include "doubleprop.h"

#define tr(str) simulideTr("ShieldSubc",str)

ShieldSubc::ShieldSubc( QString type, QString id )
          : BoardSubc( type, id )
{
    m_subcType = Chip::Shield;
    m_attached = false;
    m_boardId = "";
    m_board = NULL;
    setZValue( 1 );

    addPropGroup( {"Hidden", {
new StrProp<ShieldSubc>( "BoardId", "","", this, &ShieldSubc::boardId, &ShieldSubc::setBoardId )
    }, groupHidden} );
}
ShieldSubc::~ShieldSubc(){}

void ShieldSubc::remove()
{
    //if( m_board ) return;
    SubCircuit::remove();
}

void ShieldSubc::setBoard( BoardSubc* board )
{
    if( board )
    {
        m_boardId = board->getUid();
        board->attachShield( this );
        Circuit::self()->compList()->removeOne( this );
    }else{
        m_boardId = "";
        m_board->detachShield( this );
        Circuit::self()->compList()->append( this );
    }
    setParentItem( board );
    m_attached = board ? true : false;
    m_board = board;
}

void ShieldSubc::connectBoard()
{
    //slotAttach();return;
    m_board = NULL;
    if( m_boardId == "" ) return;

    QString name = Circuit::self()->origId( m_boardId );
    if( name != "" ) m_boardId = name;

    Component* comp = Circuit::self()->getCompById( m_boardId );
    if( comp && comp->itemType() == "Subcircuit" )
    {
        m_board = static_cast<BoardSubc*>(comp);
        attachToBoard();
        setBoard( m_board );
}   }

void ShieldSubc::slotAttach()
{
    double myZ = this->zValue();

    QList<QGraphicsItem*> list = this->collidingItems();
    QList<BoardSubc*> boardList;
    for( QGraphicsItem* it : list )
    {
        if( it->type() != UserType+1 ) continue;             // Not a Component

        Component* comp = qgraphicsitem_cast<Component*>( it );
        if( !(comp->itemType() == "Subcircuit") ) continue;  // Not a Subcircuit

        BoardSubc* board = (BoardSubc*)comp;
        if( board->subcType() < Board ) continue;            // Not a Board

        if( board->subcType() > Board ){
            ShieldSubc* shield = static_cast<ShieldSubc*>(board);
            if( m_shields.contains( shield ) ) continue;     // Is my child
        }
        double itZ = it->zValue();
        if( itZ == myZ ){
            int i=0;
            for( Component* comp : *Circuit::self()->compList() ){
                if     ( comp == this  ) myZ = i;
                else if( comp == board ) itZ = i;
                i++;
            }
        }
        if( itZ > myZ ) continue;                            // Is above me

        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        /// FIXME UNDOREDO: Circuit::self()->saveState();

        m_circPos = this->pos();
        m_board = board;           // The one with highest z value that is behind me
        attachToBoard();
        setBoard( board );
        this->moveTo( m_boardPos );
        break;
    }
}

void ShieldSubc::attachToBoard()
{
    int origX = 8*(m_board->pkgWidth()-m_width)/2;
    m_boardPos = QPointF(origX, 0);
    this->setRotation(0);
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::slotDetach()
{
    if( m_board )
    {
        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        /// FIXME UNDO REDO: Circuit::self()->saveState();

        this->moveTo( this->scenePos()+QPointF( 8,-8 ) );
        setBoard( NULL );
        renameTunnels();
    }
    m_attached = false;
}

void ShieldSubc::renameTunnels()
{
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}

void ShieldSubc::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    event->accept();
    QMenu* menu = new QMenu();
    contextMenu( event, menu );
    menu->deleteLater();
}

void ShieldSubc::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
        event->accept();

        if( m_attached )
        {
            QAction* detachAction = menu->addAction( QIcon(":/detach.png"),tr("Detach") );
            QObject::connect( detachAction, &QAction::triggered, [=](){ slotDetach(); } );
        }else{
            QAction* attachAction = menu->addAction( QIcon(":/attach.png"),tr("Attach") );
            QObject::connect( attachAction, &QAction::triggered, [=](){ slotAttach(); } );
        }

        if( m_board ) m_board->contextMenu( event, menu );
        else          SubCircuit::contextMenu( event, menu );
}
