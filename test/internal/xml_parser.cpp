#include "gtest/gtest.h"
#include "internal/xml_parser.h"

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

    lms::internal::preprocessXML(doc, {"test", "notTest2", "b"});

    std::ostringstream oss;
    doc.save(oss, "", pugi::format_raw | pugi::format_no_declaration);
    ASSERT_EQ(XMLwithoutIF, oss.str());
}

TEST(xml_parser, parseModuleConfig) {
    std::string configNode(
            "<config>"
            "<group name=\"size\">"
            "<int name=\"width\">400</int>"
            "<float name=\"height\">300.10</float>"
            "<group name=\"deep\"><string name=\"down\">A B\n C\tD</string></group>"
            "</group>"
            "<bool name=\"flag\">true</bool>"
            "</config>");

    pugi::xml_document doc;
    std::istringstream iss(configNode);
    ASSERT_TRUE(doc.load(iss));

    lms::Config config;
    lms::internal::parseModuleConfig(doc.child("config"), config, "");

    EXPECT_EQ(400, config.get<int>("size.width", 0));
    EXPECT_FLOAT_EQ(300.10, config.get<float>("size.height", 0));
    EXPECT_TRUE(config.get<bool>("flag", false));
    EXPECT_EQ(std::vector<std::string>({"A","B","C","D"}),
              config.getArray<std::string>("size.deep.down", {}));
}

TEST(xml_parser, parseService) {
    std::istringstream xml(std::string(
        "<lms>"
        "<service name=\"test\" lib=\"test_service\" class=\"TestService\" log=\"warn\">"
        "</service>"
        "</lms>"));
    lms::internal::RuntimeInfo info;
    lms::internal::XmlParser parser(info);
    parser.parseFile(xml, "");

    ASSERT_EQ(0u, parser.errors().size());

    ASSERT_EQ(1u, info.services.size());
    EXPECT_EQ("test",  info.services[0].name);
    EXPECT_EQ("test_service", info.services[0].lib);
    EXPECT_EQ("TestService", info.services[0].clazz);
    EXPECT_EQ(lms::logging::Level::WARN, info.services[0].log);
}

TEST(xml_parser, parseModule) {
    std::istringstream xml(std::string(
        "<lms>"
        "<module name=\"my_mod\" lib=\"my_module\" class=\"MyModule\" log=\"info\">"
        "<channelHint name=\"A\" mapTo=\"B\" />"
        "<channelHint name=\"C\" mapTo=\"D\" priority=\"3\" />"
        "<channelHint name=\"E\" priority=\"7\" />"
        "</module>"
        "</lms>"
    ));
    lms::internal::RuntimeInfo info;
    lms::internal::XmlParser parser(info);
    parser.parseFile(xml, "");

    ASSERT_EQ(0u, parser.errors().size());

    ASSERT_EQ(1u, info.modules.size());
    EXPECT_EQ("my_mod", info.modules[0].name);
    EXPECT_EQ("my_module", info.modules[0].lib);
    EXPECT_EQ("MyModule", info.modules[0].clazz);
    EXPECT_EQ(lms::logging::Level::INFO, info.modules[0].log);

    ASSERT_EQ(3u, info.modules[0].channelMapping.size());
    EXPECT_EQ("B", info.modules[0].channelMapping["A"].first);
    EXPECT_EQ(0, info.modules[0].channelMapping["A"].second);
    EXPECT_EQ("D", info.modules[0].channelMapping["C"].first);
    EXPECT_EQ(3, info.modules[0].channelMapping["C"].second);
    EXPECT_EQ("E", info.modules[0].channelMapping["E"].first);
    EXPECT_EQ(7, info.modules[0].channelMapping["E"].second);
}
