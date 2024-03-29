/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>

#include "compbase.h"
#include "circuit.h"
#include "propdialog.h"
#include "comproperty.h"

CompBase::CompBase( QString type, QString id )
{
    m_id   = id;
    m_type = type;

    m_propDialog = NULL;
}
CompBase::~CompBase()
{
    for( ComProperty* p : m_propHash.values() ) delete p;
    if( m_propDialog )
    {
        m_propDialog->setParent( NULL );
        m_propDialog->close();
        delete m_propDialog;
    }
}

void CompBase::loadProperties( QDomElement* el ) // Set properties in correct order
{
    QHash<QString, QString> properties;
    QDomNamedNodeMap atrs = el->attributes();
    for( int i=0; i<atrs.count(); ++i )
    {
        QDomAttr atr = atrs.item(i).toAttr();
        properties[atr.name()] = atr.value();
    }
    for( propGroup group : m_propGroups )
    {
        QList<ComProperty*> propList = group.propList;
        if( propList.isEmpty() ) continue;
        for( ComProperty* prop : propList )
        {
            QString pn = prop->name();
            if( !properties.contains( pn ) ) continue;
            prop->setValStr( properties.value( pn ) );
            properties.remove( pn );
        }
    }
}

void CompBase::remPropGroup( QString name )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        if( m_propGroups.at(i).name == name )
        {
            for( ComProperty* p : m_propGroups.at(i).propList ) m_propHash.remove( p->name() );
            m_propGroups.removeAt(i);
            break;
}   }   }

void CompBase::addPropGroup( propGroup pg, bool list )
{
    m_propGroups.append( pg );

    if( list )
        for( ComProperty* p : pg.propList ) m_propHash[p->name()] = p;
}

void CompBase::addProperty( QString group, ComProperty* p )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup pg = m_propGroups.at(i);
        if( pg.name != group ) continue;

        pg.propList.append( p );
        m_propGroups.replace( i, pg );
        m_propHash[p->name()] = p;
        return;
}   }

void CompBase::removeProperty( QString prop )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup pg = m_propGroups.at(i);
        for( ComProperty* p : pg.propList )
        {
            if( p->name() != prop ) continue;
            pg.propList.removeAll( p );
            m_propGroups.replace( i, pg );
            m_propHash.remove( prop );
            delete p;
            return;
}   }   }

bool CompBase::setPropStr( QString prop, QString val )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ){
        p->setValStr( val );
        if( m_propDialog ) m_propDialog->updtValues();
    }
    else return false;
    return true;
}
QString CompBase::getPropStr( QString prop )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ) return p->getValStr();
    return "";
}

QString CompBase::toString() // Used to save circuit
{
    QString item = "\n<item ";
    for( propGroup pg : m_propGroups )
    {
        if( !Circuit::self()->getBoard() )     // Not a Subcircit Board
        {
            if( pg.name == "Board") continue;  // Don't save Board properties
        }

        for( ComProperty* prop : pg.propList )
        {
            QString val = prop->toString();
            if( val.isEmpty() ) continue;
            item += prop->name() + "=\""+val+"\" ";
    }   }
    item += "/>\n";

    return item;
}

int CompBase::getEnumIndex( QString prop )
{
    bool ok = false;
    int index = prop.toInt(&ok); // OLd TODELETE
    if( !ok ) index = m_enumUids.indexOf( prop );
    if( index < 0 || index > m_enumUids.size()-1) index = 0;
    return index;
}
