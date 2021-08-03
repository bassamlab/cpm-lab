# Welche Daten sind interessant?
- Participant QoS
- DataWriter QoS
- Topic Type & Object
- Topic Name

# Daten zum Erstellen eines Topic
- TopicDescription

# Daten zum Erstellen eines Participant
- Der Participant wird nicht automatisch erstellt, im Callback befindet sich lediglich der Participant, der den anderen Participant entdeckt hat
- Wichtig sind hier also die QoS: Diese kann man jedoch nicht vernünftig auslesen
- Daher zurzeit: Nutze 'default' Participant 

# Daten zum Erstellen eines Subscribers
- Nutze on_publisher_discovery (Wir wollen zu jedem Publisher einen Subscriber erstellen)
- QoS können ausgelesen werden aus: ```info.info.m_qos``` von ```eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info```
- Hier kann auch der Participant abgespeichert werden
- Zum Datentyp: Zuerst liest man aus info (wie oben):
```C++
std::cout << "New DataWriter publishing under topic '" << info.info.topicName() << "' of type '" << info.info.typeName() << "' discovered" << std::endl;
```
- Dann kann man wie im Abschnitt zum Erstellen eines Topics erläutert ein Topic erstellen

# Daten zum Erstellen eines DataListeners
- Gibt es so nicht. Für QoS: Am besten on_requested_incompatible_qos nutzen, um QoS im Nachhinein anzupassen