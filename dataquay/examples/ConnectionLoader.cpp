/*!!! pulled out from ObjectLoader.cpp: can we do this using a callback, as an example?
void
ObjectLoader::D::loadConnections(NodeObjectMap &map)
{
    QString slotTemplate = SLOT(xxx());
    QString signalTemplate = SIGNAL(xxx());

    // The store does not necessarily know m_relationshipPrefix

    ResultSet rs = m_s->query
        (QString
         (" PREFIX rel: <%1> "
          " SELECT ?sobj ?ssig ?tobj ?tslot WHERE { "
          " ?conn a rel:Connection; rel:source ?s; rel:target ?t. "
          " ?s rel:object ?sobj; rel:signal ?ssig. "
          " ?t rel:object ?tobj; rel:slot ?tslot. "
          " } ").arg(m_tm.getRelationshipPrefix().toString()));

    foreach (Dictionary d, rs) {

        Uri sourceUri(d["sobj"].value);
        Uri targetUri(d["tobj"].value);
        if (!map.contains(sourceUri) || !map.contains(targetUri)) continue;

        QString sourceSignal = signalTemplate.replace("xxx", d["ssig"].value);
        QString targetSlot = slotTemplate.replace("xxx", d["tslot"].value);

        QByteArray sigba = sourceSignal.toLocal8Bit();
        QByteArray slotba = targetSlot.toLocal8Bit();
                
        QObject::connect(map[sourceUri], sigba.data(),
                         map[targetUri], slotba.data());
    }
}
*/
