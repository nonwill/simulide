/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "function.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "utils.h"

static const char* Function_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Functions")
};

Component* Function::construct( QObject* parent, QString type, QString id )
{
    return new Function( parent, type, id );
}

LibraryItem* Function::libraryItem()
{
    return new LibraryItem(
    tr( "Function" ),
    tr( "Logic/Arithmetic" ),
    "subc.png",
    "Function",
    Function::construct );
}

Function::Function( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eElement( id )
        , m_engine()
        , m_functions()
{
    Q_UNUSED( Function_properties );

    m_lastDir = Circuit::self()->getFileName();
    
    setNumInps( 2 );                           // Create Input Pins
    setNumOuts( 1 );
    
    setFunctions( "i0 | i1" );
}
Function::~Function(){}

QList<propGroup_t> Function::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Num_Inputs", tr("Input Size"),"Inputs"} );
    mainGroup.propList.append( {"Num_Outputs", tr("Output Size"),"Outputs"} );
    mainGroup.propList.append( {"Functions", tr("Functions"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void Function::stamp()
{
    ///LogicComponent::stamp();

    for( int i=0; i<m_numInputs; ++i )
    {
        eNode* enode = m_inPin[i]->getEnode();
        if( enode ) enode->voltChangedCallback( this );
    }
    m_program.clear();
    for( int i=0; i<m_numOutputs; ++i )
    {
        m_program.append( QScriptProgram( m_funcList.at(i) ));
    }
}

void Function::voltChanged()
{
    //uint bits = 0;
    uint bit = 0;
    //uint msb = (m_numInputs+m_numOutputs)*2-1;
    for( int i=0; i<m_numInputs; ++i )
    {
        bit = m_inPin[i]->getInpState();
        //if( bit ) bits += 1 << (msb-(i*4));
        //else      bits += 1 << (msb-(i*4)-1);
        m_engine.globalObject().setProperty( "i"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vi"+QString::number(i), QScriptValue( m_inPin[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "inBits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "inputs", QScriptValue( m_numInputs ) );

    for( int i=0; i<m_numOutputs; ++i )
    {
        bit = m_outPin[i]->getOutState();
        //if( bit ) bits += 1 << (msb-(i*4)-2);
        //else      bits += 1 << (msb-(i*4)-3);
        m_engine.globalObject().setProperty( "o"+QString::number(i), QScriptValue( bit ) );
        m_engine.globalObject().setProperty( "vo"+QString::number(i), QScriptValue( m_outPin[i]->getVolt()) );
    }
    //m_engine.globalObject().setProperty( "bits", QScriptValue( bits ) );
    //m_engine.globalObject().setProperty( "outputs", QScriptValue( m_numOutputs ) );

    m_nextOutVal = 0;
    for( int i=0; i<m_numOutputs; ++i )
    {
        if( i >= m_numOutputs ) break;
        QString text = m_funcList.at(i).toLower();

        //qDebug() << "eFunction::voltChanged()"<<text<<m_engine.evaluate( text ).toString();

        if( text.startsWith( "vo" ) )
        {
            float out = m_engine.evaluate( m_program.at(i) ).toNumber();
            m_outPin[i]->setOutHighV( out );
            m_nextOutVal += 1<<i;
        }
        else
        {
            bool out = m_engine.evaluate( m_program.at(i) ).toBool();
            m_outPin[i]->setOutHighV( m_ouHighV );
            if( out ) m_nextOutVal += 1<<i;
        }
    }
    sheduleOutPuts( this );
}

QString Function::functions()
{
    return m_functions;
}

void Function::setFunctions( QString f )
{
    //qDebug()<<"eFunction::setFunctions"<<f;
    if( f.isEmpty() ) return;
    m_functions = f;
    m_funcList = f.split(",");
}

void Function::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
    }
}

void Function::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    menu->addSeparator();
    QAction* loadDaAction = menu->addAction( QIcon(":/load.png"),tr("Load Functions") );
    connect( loadDaAction, SIGNAL(triggered()),
                     this, SLOT(loadData()), Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save Functions") );
    connect( saveDaAction, SIGNAL(triggered()),
                     this, SLOT(saveData()), Qt::UniqueConnection );
    menu->addSeparator();
}

void Function::loadData()
{
    QString fileName = QFileDialog::getOpenFileName( 0l, "Function::loadData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels loading
    m_lastDir = fileName;
    QStringList lines = fileToStringList( fileName, "MemData::loadData" );

    int i=0;
    for( QString line : lines )
    {
        if( line.remove(" ").isEmpty() ) continue;
        if( i >= m_funcList.size() ) break;
        m_funcList[i++] = line;
        m_functions = m_funcList.join(",");
    }
}

void Function::saveData()
{
    QString fileName = QFileDialog::getSaveFileName( 0l, "Function::saveData", m_lastDir, "" );

    if( fileName.isEmpty() ) return; // User cancels saving
    m_lastDir = fileName;
    QFile outFile( fileName );
    QString output = "";
    for( QString func : m_funcList ) output.append( func+"\n");

    if( !outFile.open( QFile::WriteOnly | QFile::Text ) )
    {
          QMessageBox::warning( 0l, "MemData::saveData",
          QCoreApplication::translate( "MemData", "Cannot write file %1:\n%2.").arg(fileName).arg(outFile.errorString()));
    }else {
        QTextStream toFile( &outFile );
        toFile << output;
        outFile.close();
    }
}

void Function::remove()
{
    for( QPushButton* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       delete button;
    }
    LogicComponent::remove();
}

void Function::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;
    
    if( inputs < m_numInputs ) 
    {
        int dif = m_numInputs-inputs;

        LogicComponent::deleteInputs( dif );
    }
    else{
        m_inPin.resize( inputs );
        m_numInputs = inputs;
    
        for( int i=m_numInputs; i<inputs; ++i )
        {
            QString num = QString::number(i);
            m_inPin[i] = new IoPin( 180, QPoint(-24, i*8+8 ), m_id+"-in"+num, i, this,input );
            m_inPin[i]->setLabelText( " I"+num );
            m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        }
    }
    m_height = m_numOutputs*2-1;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );
    
    Circuit::self()->update();
}

void Function::setNumOuts( int outs )
{
    if( outs == m_numOutputs ) return;
    if( outs < 1 ) return;
    
    if( outs < m_numOutputs ) 
    {
        int dif = m_numOutputs-outs;

        LogicComponent::deleteOutputs( dif );
    
        for( int i=0; i<dif; ++i )
        {
            QPushButton* button = m_buttons.takeLast();
            disconnect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
            delete button;
            
            m_proxys.removeLast();
            m_funcList.removeLast();
        }
    }else{
        m_outPin.resize( outs );
        m_numOutputs = outs;
        
        for( int i=m_numOutputs; i<outs; ++i )
        {
            QString num = QString::number(i);
            m_outPin[i] = new IoPin( 0, QPoint(24, i*8*2+8 ), m_id+"-out"+num, i, this, output );
            
            QPushButton* button = new QPushButton( );
            button->setMaximumSize( 14,14 );
            button->setGeometry(-14,-14,14,14);
            QFont font = button->font();
            font.setPixelSize(7);
            button->setFont(font);
            button->setText( "O"+num );
            button->setCheckable( true );
            m_buttons.append( button );

            QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( button );
            proxy->setParentItem( this );
            proxy->setPos( QPoint( 0, i*8*2+1 ) );
            
            m_proxys.append( proxy );
            m_funcList.append( "" );
            
            connect( button, SIGNAL( released() ),
                       this, SLOT  ( onbuttonclicked() ), Qt::UniqueConnection );
        }
    }
    m_height = m_numOutputs*2-1;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );
    
    m_functions = m_funcList.join(",");
    
    Circuit::self()->update();
}

void Function::onbuttonclicked()
{
    int i = 0;
    for( QPushButton* button : m_buttons ) 
    {
       if( button->isChecked()  )
       {
           button->setChecked( false );
           break;
       }
       ++i;
    }
    bool ok;
    QString text = QInputDialog::getText(0l, tr("Set Function"),
                                             "Output "+QString::number(i)+tr(" Function:"), 
                                             QLineEdit::Normal,
                                             m_funcList[i], &ok);
    if( ok && !text.isEmpty() )
    {
        m_funcList[i] = text;
        m_functions = m_funcList.join(",");
    }
}

#include "moc_function.cpp"
