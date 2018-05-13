#include "defs.h"
#include "rapidjson/document.h"
using namespace rapidjson;

#include "utils.h"

#include <QtTest>
#include <QAbstractSocket>

#define AssertTrue(statement, message) QVERIFY2(statement, message)
#define AssertFalse(statement, message) QVERIFY2(!(statement), message)

#define AssertEquals(expected, actual) \
do {\
    if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__))\
        return;\
} while (false)

class GDaxTestsTest : public QObject
{
    Q_OBJECT

public:
    GDaxTestsTest(){}

private slots:
    void InitTestCase(){}
    void CleanupTestCase(){}

    void ParsingL2Update()
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
        AssertEquals(1, changesArray.size());
        QJsonValueRef change = changesArray[0];
        AssertTrue(change.isArray(), "change is array");

        QJsonArray subArray = change.toArray();
        AssertEquals(3, subArray.size());

        AssertTrue(subArray[0].isString(), "sub0 isString");
        AssertEquals("sell", subArray[0].toString());
        AssertTrue(subArray[1].isString(), "ar1 is string");
        AssertEquals("7202.91000000", subArray[1].toString());
        AssertTrue(subArray[2].isString(), "ar2 is string");
        AssertEquals("5.62802151", subArray[2].toString());
    }

    void ParsingHeartbeat()
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

    void UsingRapidJsonL2Update()
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

    void RapidJsonHeartbeat()
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

    void RapidJsonTicker()
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

        /* also can get....
//     "time": "2017-09-02T17:05:49.250000Z",
//     "price": "4388.01000000",
//     "side": "buy",
//      [open_24h, volume_24h, low_24h, high_24h, volume_30d <--
//     "last_size": "0.03000000",
//     "best_bid": "4388",
//     "best_ask": "4388.01"
*/

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

    void ParseCandles()
    {
        QString data = R"([[1525450800,8035.81,8035.82,8035.82,8035.81,0.23243],
[1525450740,8020,8046.32,8046.32,8035.82,7.87281723],
[1525450680,8046.31,8046.32,8046.31,8046.31,0.44734494]])";

        QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
        //AssertTrue(document.isObject(), "isObject");
        AssertTrue(document.isArray(), "isArray");
        auto array = document.array();
        AssertEquals(3, array.size());

        const auto & ar1 = array[0].toArray();
        AssertEquals(6, ar1.size());
        const auto & ar2 = array[1].toArray();
        AssertEquals(6, ar2.size());
        const auto & ar3 = array[2].toArray();
        AssertEquals(6, ar3.size());

        for(int i=0; i<6;++i)
        {
            AssertTrue(ar1[1].isDouble(), "isDouble");
            AssertTrue(ar2[1].isDouble(), "isDouble");
            AssertTrue(ar3[1].isDouble(), "isDouble");
        }
    }

    void Deltas()
    {
        AssertEquals("", DiffText("", ""));
        AssertEquals("", DiffText("abc", ""));
        AssertEquals(R"(<span>abc</span>)", DiffText("abc", "abc"));
        AssertEquals(R"(abc)", DiffText("", "abc"));
        AssertEquals(R"(def)", DiffText("abc", "def"));
        AssertEquals(R"(<span>a</span>xc)", DiffText("abc", "axc"));
        AssertEquals(R"(xbc)", DiffText("abc", "xbc"));
        AssertEquals(R"(<span>ab</span>x)", DiffText("abc", "abx"));
        AssertEquals(R"(<span>abc</span>d)", DiffText("abc", "abcd"));
        AssertEquals(R"(<span>a</span>BcDe)", DiffText("abcde", "aBcDe"));
    }

    void BaadPrice()
    {
        DecNs::decimal<8> dp8("99999999");
        AssertEquals(99999999, dp8.getAsXDouble());

        DecNs::decimal<8> dp9("999999999");
        AssertEquals(999999999, dp9.getAsXDouble());

        DecNs::decimal<8> dp10("9999999999");
        AssertTrue(9999999999 != dp9.getAsXDouble(), "10 digits blows decimal");
    }

    void EnumToSting()
    {
        AssertEquals("SocketTimeoutError", QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(QAbstractSocket::SocketError::SocketTimeoutError));
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

/*
candles
https://api-public.sandbox.gdax.com/products/BTC-EUR/candles
https://api.gdax.com//products/BTC-EUR/candles

start	Start time in ISO 8601
end	End time in ISO 8601

granularity	Desired timeslice in seconds
60, 300, 900, 3600, 21600, 86400

current time
1525451245

start time
lowest price
highest price
opening price
closing price
volume

def=1minute?
[1525450800,8035.81,8035.82,8035.82,8035.81,0.23243],
[1525450740,8020,8046.32,8046.32,8035.82,7.87281723],
[1525450680,8046.31,8046.32,8046.31,8046.31,0.44734494],
[1525450620,8046.31,8046.32,8046.31,8046.32,0.48998061000000004],
[1525450560,8046.31,8046.32,8046.31,8046.32,0.28],
[1525450500,8029.5,8046.32,8029.5,8046.32,0.55522599],
[1525450440,8024.95,8029.5,8024.96,8029.5,1.1415757199999998],
[1525450380,8024.95,8024.96,8024.96,8024.95,5.38000906],
[1525450320,8024.95,8024.96,8024.96,8024.96,0.28],
[1525450260,8022.02,8024.95,8022.02,8024.95,3.278330829999999],
[1525450200,8022.01,8022.02,8022.01,8022.02,0.22536186],
[1525450140,8022,8022.01,8022.01,8022.01,0.11],
[1525450080,8022,8023.79,8022.5,8022,1.3400000000000003],
[1525450020,8022,8024.95,8024.95,8023.79,0.5218723700000001],
[1525449960,8023.04,8024.95,8024.8,8024.95,0.3533545],
[1525449900,8017.1,8025,8021.73,8024.8,18.464182259999994],
[1525449840,8021.7,8037.54,8024.52,8021.72,4.46623744],
[1525449780,8021.7,8041.77,8030.05,8021.7,3.61542692],
[1525449720,8024,8050.01,8050.01,8030.02,8.64174912],

+gran
[1525449600,8017.1,8058.39,8058.39,8058.34,61.48445708999999],
[1525446000,8031.06,8120,8109.75,8058.39,109.91127743000035],
[1525442400,8068.46,8112.55,8089.47,8103.08,119.06663868000032],
[1525438800,8077,8140,8092.82,8089.47,92.0301390400001],
[1525435200,8089,8153.21,8131.01,8092.82,124.81411288000031],
[1525431600,8070,8150,8100.02,8131,134.7601269000004],
[1525428000,8100.02,8130.45,8127.01,8100.02,83.32251286000022],
[1525424400,8047.88,8166,8153.31,8127.01,147.63549509000038],
[1525420800,8072.22,8160,8073.29,8153.3,160.10459283000043],
[1525417200,8052.46,8096.33,8081,8073.3,81.95595686000027],
[1525413600,8003.61,8097.73,8015.97,8081.01,75.16895106000011],
[1525410000,8011.21,8049.9,8035.74,8015.97,38.33417047000007],
*/

QTEST_APPLESS_MAIN(GDaxTestsTest)

#include "tst_gdaxteststest.moc"
