#include <QString>
#include <QtTest>

#include "rapidjson/document.h"
using namespace rapidjson;

#define AssertTrue(statement, message) QVERIFY2(statement, message)
#define AssertFalse(statement, message) QVERIFY2(!(statement), message)
#define AssertEquals(expected, actual) QCOMPARE(actual, expected)

class GDaxTestsTest : public QObject
{
    Q_OBJECT

public:
    GDaxTestsTest(){}

private Q_SLOTS:
    void initTestCase(){}
    void cleanupTestCase(){}

    void testParsingL2Update()
    {
        QString data(R"({
    "type":"l2update",
    "product_id":"BTC-EUR",
    "time":"2018-04-23T01:26:56.035Z",
    "changes":[["sell","7202.91000000","5.62802151"]]
})");

        QJsonDocument document(QJsonDocument::fromJson(data.toUtf8()));
        QJsonObject object = document.object();

        AssertTrue(document.isObject(), "isObject");
        AssertTrue(object.contains("type"), "contains");
        AssertTrue(object["type"].isString(), "typeIsString");
        AssertEquals(object["type"].toString(), "l2update");

        AssertEquals("l2update", object["type"].toString());
        AssertEquals("BTC-EUR", object["product_id"].toString());
        AssertEquals("2018-04-23T01:26:56.035Z", object["time"].toString());

        QJsonValueRef changes = object["changes"];
        AssertTrue(changes.isArray(), "isArray");
        QJsonArray changesArray = changes.toArray();
        AssertEquals(1u, changesArray.size());
        QJsonValueRef change = changesArray[0];
        AssertTrue(change.isArray(), "change is array");

        QJsonArray subArray = change.toArray();
        AssertEquals(3u, subArray.size());

        AssertTrue(subArray[0].isString(), "sub0 isString");
        AssertEquals("sell", subArray[0].toString());
        AssertTrue(subArray[1].isString(), "ar1 is string");
        AssertEquals("7202.91000000", subArray[1].toString());
        AssertTrue(subArray[2].isString(), "ar2 is string");
        AssertEquals("5.62802151", subArray[2].toString());
    }

    void testParsingHeartbeat()
    {
        QString data(R"({
    "type":"heartbeat",
    "last_trade_id":14275328,
    "product_id":"BTC-EUR",
    "sequence":3637742500,
    "time":"2018-05-03T19:29:24.335000Z"
})");

        QJsonDocument document(QJsonDocument::fromJson(data.toUtf8()));
        QJsonObject object = document.object();

        AssertTrue(document.isObject(), "isObject");
        AssertTrue(object.contains("type"), "contains type");
        AssertTrue(object["type"].isString(), "typeIsString");
        AssertEquals(object["type"].toString(), "heartbeat");

        AssertTrue(object.contains("last_trade_id"), "contains tradeId");
        AssertEquals(object["last_trade_id"].type(), QJsonValue::Type::Double);
    }

    void testUsingRapidJsonL2Update()
    {
        Document document;
        document.Parse(R"({
        "type":"l2update",
        "product_id":"BTC-EUR",
        "time":"2018-04-23T01:26:56.035Z",
        "changes":[["sell","7202.91000000","5.62802151"]]
    })");

        AssertTrue(document.IsObject(), "");
        AssertTrue(document.HasMember("type"), "");
        AssertTrue(document["type"].IsString(), "");
        AssertEquals("l2update", document["type"].GetString());
        AssertEquals("BTC-EUR", document["product_id"].GetString());
        AssertEquals("2018-04-23T01:26:56.035Z", document["time"].GetString());

        const Document::ValueType & changes = document["changes"];
        AssertTrue(changes.IsArray(), "");
        AssertEquals(1u, changes.Size());
        const auto & array = changes[0];
        AssertEquals(3u, array.Size());

        AssertTrue(array[0].IsString(), "");
        AssertEquals("sell", array[0].GetString());
        AssertFalse(array[1].IsNumber(), "");
        AssertEquals("7202.91000000", array[1].GetString());
        AssertFalse(array[2].IsNumber(), "");
        AssertEquals("5.62802151", array[2].GetString());
    }

    void testRapidJsonHeartbeat()
    {
        std::string data(R"({
    "type":"heartbeat",
    "last_trade_id":14275328,
    "product_id":"BTC-EUR",
    "sequence":3637742500,
    "time":"2018-05-03T19:29:24.335000Z"
})");

        Document document;
        document.Parse(data.c_str());

        AssertTrue(document.IsObject(), "isObject");
        const auto & object = document.GetObject();
        AssertTrue(object.HasMember("type"), "contains type");
        AssertTrue(object["type"].IsString(), "typeIsString");
        AssertEquals(object["type"].GetString(), "heartbeat");

        AssertTrue(object.HasMember("last_trade_id"), "contains tradeId");
        AssertTrue(object["last_trade_id"].IsNumber(), "tradeId is number");
        AssertEquals(object["last_trade_id"].GetUint64(), 14275328ull);

        AssertTrue(object.HasMember("sequence"), "contains sequence");
        AssertTrue(object["sequence"].IsNumber(), "sequence is number");
        AssertTrue(object["sequence"].IsUint64(), "sequence is number");
        AssertEquals(object["sequence"].GetUint64(), 3637742500ull);
    }

    void testRapidJsonTicker()
    {
        std::string data(R"({
    "type": "ticker",
    "trade_id": 20153558,
    "sequence": 3262786978,
    "time": "2017-09-02T17:05:49.250000Z",
    "product_id": "BTC-USD",
    "price": "4388.01000000",
    "side": "buy",
    "last_size": "0.03000000",
    "best_bid": "4388",
    "best_ask": "4388.01"
})");

        Document document;
        document.Parse(data.c_str());

        AssertTrue(document.IsObject(), "isObject");
        const auto & object = document.GetObject();
        AssertTrue(object.HasMember("type"), "contains type");
        AssertTrue(object["type"].IsString(), "typeIsString");
        AssertEquals(object["type"].GetString(), "ticker");

        AssertTrue(object.HasMember("trade_id"), "contains tradeId");
        AssertTrue(object["trade_id"].IsNumber(), "tradeId is number");
        AssertTrue(object["trade_id"].IsUint64(), "sequence is number");
        AssertEquals(object["trade_id"].GetUint64(), 20153558ull);

        AssertTrue(object.HasMember("sequence"), "contains sequence");
        AssertTrue(object["sequence"].IsNumber(), "sequence is number");
        AssertTrue(object["sequence"].IsUint64(), "sequence is number");
        AssertEquals(object["sequence"].GetUint64(), 3262786978ull);

        AssertEquals(object["time"].GetString(), "2017-09-02T17:05:49.250000Z");
        AssertEquals(object["product_id"].GetString(), "BTC-USD");
        AssertEquals(object["price"].GetString(), "4388.01000000");
        AssertEquals(object["side"].GetString(), "buy");
        AssertEquals(object["last_size"].GetString(), "0.03000000");
        AssertEquals(object["best_bid"].GetString(), "4388");
        AssertEquals(object["best_ask"].GetString(), "4388.01");
    }
};

/*
 {
    "type": "error",
    "message": "error message",
 }

-- can have dropped messages, use hb\rest
{
    "type": "match",
    "trade_id": 10,
    "sequence": 50,
    "maker_order_id": "ac928c66-ca53-498f-9c13-a110027a60e8",
    "taker_order_id": "132fb6ae-456b-4654-b4e0-d681ac05cea1",
    "time": "2014-11-07T08:19:27.028459Z",
    "product_id": "BTC-USD",
    "size": "5.23512",
    "price": "400.23",
    "side": "sell"
}
*/

QTEST_APPLESS_MAIN(GDaxTestsTest)

#include "tst_gdaxteststest.moc"
