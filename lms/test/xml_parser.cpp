#include "gtest/gtest.h"
#include "lms/xml_parser.h"

TEST(xml_parser, preprocessXML) {
    std::string XMLwithIF(
            "<framework>"
            "<modulesToEnable>"
            "<if set=\"test\"><module>Test</module></if>"
            "<if notSet=\"notTest2\"><module>Test2</module></if>"
            "</modulesToEnable>"
            "<module>"
            "<name>Test</name>"
            "<if anyOf=\"a,b\"><realName>Test2</realName></if>"
            "<if allOf=\"test,b\"><config><i>10</i></config></if>"
            "</module>"
            "</framework>");

    std::string XMLwithoutIF(
            "<framework>"
            "<modulesToEnable>"
            "<module>Test</module>"
            "</modulesToEnable>"
            "<module>"
            "<name>Test</name>"
            "<realName>Test2</realName>"
            "<config><i>10</i></config>"
            "</module>"
            "</framework>");

    pugi::xml_document doc;
    std::istringstream iss(XMLwithIF);
    ASSERT_TRUE(doc.load(iss));

    lms::preprocessXML(doc, {"test", "notTest2", "b"});

    std::ostringstream oss;
    doc.save(oss, "", pugi::format_raw | pugi::format_no_declaration);
    ASSERT_EQ(XMLwithoutIF, oss.str());
}

TEST(xml_parser, parseModuleConfig) {
    std::string configNode(
            "<config>"
            "<size>"
            "<width>400</width>"
            "<height>300.10</height>"
            "<deep><down>A,B,C</down></deep>"
            "</size>"
            "<flag>true</flag>"
            "</config>");

    pugi::xml_document doc;
    std::istringstream iss(configNode);
    ASSERT_TRUE(doc.load(iss));

    lms::type::ModuleConfig config;
    lms::parseModuleConfig(doc.child("config"), config, "");

    EXPECT_EQ(400, config.get<int>("size.width"));
    EXPECT_FLOAT_EQ(300.10, config.get<float>("size.height"));
    EXPECT_TRUE(config.get<bool>("flag"));
    EXPECT_EQ(std::vector<std::string>({"A","B","C"}),
              config.getArray<std::string>("size.deep.down"));
}
