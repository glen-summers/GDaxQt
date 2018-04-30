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

QTEST_APPLESS_MAIN(GDaxTestsTest)

#include "tst_gdaxteststest.moc"
