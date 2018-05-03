#include <QString>
#include <QtTest>

class GDaxTestsTest : public QObject
{
    Q_OBJECT

public:
    GDaxTestsTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();
    void testCase2();
    void testCase3();
    void testCase4();
};

GDaxTestsTest::GDaxTestsTest()
{
}

void GDaxTestsTest::initTestCase()
{
}

void GDaxTestsTest::cleanupTestCase()
{
}

// reverse compare
void GDaxTestsTest::testCase1()
{
    QString data(R"({"type":"l2update","product_id":"BTC-EUR","time":"2018-04-23T01:26:56.035Z","changes":[["sell","7202.91000000","5.62802151"]]})");

    QJsonDocument document(QJsonDocument::fromJson(data.toUtf8()));
    QJsonObject object = document.object();

    QVERIFY2(document.isObject(), "isObject");
    QVERIFY2(object.contains("type"), "contains");
    QVERIFY2(object["type"].isString(), "typeIsString");
    QCOMPARE(object["type"].toString(), "l2update");

    QCOMPARE("l2update", object["type"].toString());
    QCOMPARE("BTC-EUR", object["product_id"].toString());
    QCOMPARE("2018-04-23T01:26:56.035Z", object["time"].toString());

    QJsonValueRef changes = object["changes"];
    QVERIFY2(changes.isArray(), "isArray");
    QJsonArray changesArray = changes.toArray();
    QCOMPARE(1u, changesArray.size());
    QJsonValueRef change = changesArray[0];
    QVERIFY2(change.isArray(), "change is array");

    QJsonArray subArray = change.toArray();
    QCOMPARE(3u, subArray.size());

    QVERIFY2(subArray[0].isString(), "sub0 isString");
    QCOMPARE("sell", subArray[0].toString());
    QVERIFY2(subArray[1].isString(), "ar1 is string");
    QCOMPARE("7202.91000000", subArray[1].toString());
    QVERIFY2(subArray[2].isString(), "ar2 is string");
    QCOMPARE("5.62802151", subArray[2].toString());
}

void GDaxTestsTest::testCase2()
{
    QString data(R"({"type":"heartbeat","last_trade_id":14275328,"product_id":"BTC-EUR","sequence":3637742500,"time":"2018-05-03T19:29:24.335000Z"})");

    QJsonDocument document(QJsonDocument::fromJson(data.toUtf8()));
    QJsonObject object = document.object();

    QVERIFY2(document.isObject(), "isObject");
    QVERIFY2(object.contains("type"), "contains type");
    QVERIFY2(object["type"].isString(), "typeIsString");
    QCOMPARE(object["type"].toString(), "heartbeat");

    QVERIFY2(object.contains("last_trade_id"), "contains tradeId");
    QCOMPARE(object["last_trade_id"].type(), QJsonValue::Type::Double);
}

// try rj...
#include "rapidjson/document.h"
using namespace rapidjson;

void GDaxTestsTest::testCase3()
{
    Document document;
    document.Parse(R"({"type":"l2update","product_id":"BTC-EUR","time":"2018-04-23T01:26:56.035Z","changes":[["sell","7202.91000000","5.62802151"]]})");
    /*
     *{
     *"type":"l2update",
     *"product_id":"BTC-EUR",
     *"time":"2018-04-23T01:26:56.035Z",
     *"changes":[["sell","7202.91000000","5.62802151"]]
     *}
    */

    QVERIFY2(document.IsObject(), "");
    QVERIFY2(document.HasMember("type"), "");
    QVERIFY2(document["type"].IsString(), "");
    QCOMPARE("l2update", document["type"].GetString());
    QCOMPARE("BTC-EUR", document["product_id"].GetString());
    QCOMPARE("2018-04-23T01:26:56.035Z", document["time"].GetString());

    const Document::ValueType & changes = document["changes"];
    QVERIFY2(changes.IsArray(), "");
    QCOMPARE(1u, changes.Size());
    const auto & array = changes[0];
    QCOMPARE(3u, array.Size());

    QVERIFY2(array[0].IsString(), "");
    QCOMPARE("sell", array[0].GetString());
    QVERIFY2(!array[1].IsNumber(), "");
    QCOMPARE("7202.91000000", array[1].GetString());
    QVERIFY2(!array[2].IsNumber(), "");
    QCOMPARE("5.62802151", array[2].GetString());
}

void GDaxTestsTest::testCase4()
{
    std::string data(R"({"type":"heartbeat","last_trade_id":14275328,"product_id":"BTC-EUR","sequence":3637742500,"time":"2018-05-03T19:29:24.335000Z"})");

    Document document;
    document.Parse(data.c_str());

    QVERIFY2(document.IsObject(), "isObject");
    const auto & object = document.GetObject();
    QVERIFY2(object.HasMember("type"), "contains type");
    QVERIFY2(object["type"].IsString(), "typeIsString");
    QCOMPARE(object["type"].GetString(), "heartbeat");

    QVERIFY2(object.HasMember("last_trade_id"), "contains tradeId");
    QVERIFY2(object["last_trade_id"].IsNumber(), "tradeId is number");
    QCOMPARE(object["last_trade_id"].GetUint64(), 14275328ull);

    QVERIFY2(object.HasMember("sequence"), "contains sequence");
    QVERIFY2(object["sequence"].IsNumber(), "sequence is number");
    QVERIFY2(object["sequence"].IsUint64(), "sequence is number");
    QCOMPARE(object["sequence"].GetUint64(), 3637742500ull);
}

QTEST_APPLESS_MAIN(GDaxTestsTest)

#include "tst_gdaxteststest.moc"
