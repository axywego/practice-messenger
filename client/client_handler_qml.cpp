#include "client_handler_qml.hpp"

ClientHandlerQml::ClientHandlerQml(std::shared_ptr<ClientAsyncHandler> handler, QObject* parent = nullptr)
    : QObject(parent), handler(handler) { }

bool ClientHandlerQml::registerUser(const QVariantHash& data){
    if(auto p = handler.lock(); p != nullptr ) {
        AuthRequest req {
            .login = data["login"].toString().toStdString(), 
            .password = data["password"].toString().toStdString()
        };
        p->send_packet(PacketType::REGISTER, req.serialize());  
    }
}

bool loginUser(const QVariantHash& data);

bool uploadFile(const QVariantHash& data);
bool downloadFile(const QVariantHash& data);

bool sendMessageGeneralChat(const QVariantHash& data);
bool getMessageFromGeneralChat(const QVariantHash& data);

bool sendFriendRequest(const QVariantHash& data);
bool acceptFriendRequest(const QVariantHash& data);
bool rejectFriendRequest(const QVariantHash& data);
bool getFriendList(const QVariantHash& data);

QVariantHash sendDirectMessage(const QVariantHash& data);
QVariantList getHistoryDirectMessage(const QVariantHash& data);