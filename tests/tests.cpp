/****************************************************************************
 **
 ** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the jom project on Trolltech Labs.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License version 2.0 or 3.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of
 ** this file.  Please review the following information to ensure GNU
 ** General Public Licensing requirements will be met:
 ** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
 ** http://www.gnu.org/copyleft/gpl.html.
 **
 ** If you are unsure which license is appropriate for your use, please
 ** contact the sales department at qt-sales@nokia.com.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/
#include <QTest>
#include <QDir>
#include <QDebug>

#include <ppexpr/ppexprparser.h>
#include <preprocessor.h>
#include <parser.h>
#include <exception.h>

using namespace NMakeFile;

class ParserTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    // preprocessor tests
    void includeFiles();
    void includeCycle();
    void macros();
    void preprocessorExpressions_data();
    void preprocessorExpressions();
    void preprocessorDivideByZero();
    void preprocessorInvalidExpressions_data();
    void preprocessorInvalidExpressions();
    void conditionals();
    void dotDirectives();

    // parser tests
    void descriptionBlocks();
    void inferenceRules_data();
    void inferenceRules();
    void cycleInTargets();
    void dependentsWithSpace();
    void multipleTargets();
    void comments();
    void fileNameMacros();
    void fileNameMacrosInDependents();
    void windowsPathsInTargetName();

private:
    QString m_oldCurrentPath;
    Preprocessor* m_preprocessor;
};

void ParserTest::initTestCase()
{
    m_preprocessor = 0;
    m_oldCurrentPath = QDir::currentPath();
    if (QFile::exists("../makefiles"))
        QDir::setCurrent("../makefiles");
    else
        QDir::setCurrent("makefiles");
}

void ParserTest::cleanupTestCase()
{
    delete m_preprocessor;
    QDir::setCurrent(m_oldCurrentPath);
}

void ParserTest::includeFiles()
{
    MacroTable macroTable;
    Preprocessor pp;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("include_test.mk")) );
    bool exceptionCaught = false;
    try {
        while (!pp.readLine().isNull());
    } catch (Exception e) {
        qDebug() << e.message();
        exceptionCaught = true;
    }
    QVERIFY(!exceptionCaught);
    QVERIFY(macroTable.isMacroDefined("INCLUDE"));
    QCOMPARE(macroTable.macroValue("INCLUDE1"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE2"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE3"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE4"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE5"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE6"), QLatin1String("TRUE"));
    QCOMPARE(macroTable.macroValue("INCLUDE7"), QLatin1String("TRUE"));
}

void ParserTest::includeCycle()
{
    MacroTable macroTable;
    Preprocessor pp;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("circular_include.mk")) );
    bool bExceptionCaught = false;
    try {
        while (!pp.readLine().isNull());
    } catch (Exception e) {
        qDebug() << e.message();
        bExceptionCaught = true;
    }
    QVERIFY(bExceptionCaught);
}

void ParserTest::macros()
{
    MacroTable macroTable;
    Preprocessor pp;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("macrotest.mk")) );
    bool bExceptionCaught = false;
    try {
        while (!pp.readLine().isNull());
    } catch (Exception e) {
        qDebug() << e.message();
        bExceptionCaught = true;
    }
    QVERIFY(!bExceptionCaught);
    QCOMPARE(macroTable.macroValue("VERY_LONG_Macro_Name_With_mucho_mucho_characters_and_some_number_too_1234458789765421200218427824996512548989654486630110059699471421"), QLatin1String("AHA"));
    QCOMPARE(macroTable.macroValue("SEIN"), QLatin1String("ist"));
    QCOMPARE(macroTable.macroValue("vielipsum_istwallewalle_"), QLatin1String("Icke wa dsch und er denn uurrrgh..."));
    QVERIFY(macroTable.isMacroDefined("NoContent"));
    QCOMPARE(macroTable.macroValue("NoContent"), QLatin1String(""));
    QCOMPARE(macroTable.macroValue("Literal1"), QLatin1String("# who does that anyway? #"));
    QCOMPARE(macroTable.macroValue("Literal2"), QLatin1String("thi$$ i$$ pricele$$$$"));
    QCOMPARE(macroTable.expandMacros("$(Literal2)"), QLatin1String("thi$ i$ pricele$$"));
    QCOMPARE(macroTable.macroValue("Literal3"), QLatin1String("schnupsi\nwupsi\ndupsi"));
    QCOMPARE(macroTable.macroValue("Literal4"), QLatin1String("backslash at the end\\"));
    QCOMPARE(macroTable.macroValue("Literal5"), QLatin1String("backslash at the end\\"));
    QCOMPARE(macroTable.macroValue("Literal6"), QLatin1String("backslash at the end\\"));
    QCOMPARE(macroTable.macroValue("SplitOverLines"), QLatin1String("one  two three"));
    QCOMPARE(macroTable.macroValue("Incremental"), QLatin1String("one two"));
    QCOMPARE(macroTable.expandMacros(macroTable.macroValue("LateDefinition")),
             QLatin1String("_thi$ i$ pricele$$_"));
    QCOMPARE(macroTable.expandMacros("$B"), QLatin1String("B"));
    QCOMPARE(macroTable.expandMacros("$y"), QLatin1String("y"));
    QCOMPARE(macroTable.expandMacros("$(BANANA)"), QLatin1String("yellow"));
    QCOMPARE(macroTable.expandMacros("$1"), QLatin1String("x"));
    QCOMPARE(macroTable.expandMacros("$(XXX)"), QLatin1String("xxx"));
    QVERIFY(!macroTable.isMacroDefined("ThisIsNotDefined"));
}

void ParserTest::preprocessorExpressions_data()
{
    QTest::addColumn<QByteArray>("expression");
    QTest::addColumn<int>("expected");
    QTest::newRow("max number") << QByteArray("2147483647") << 2147483647;
    QTest::newRow("min number") << QByteArray("-2147483647") << -2147483647;
    QTest::newRow("file exists 1") << QByteArray("EXIST(include_test.mk)") << 1;
    QTest::newRow("file exists 2") << QByteArray("eXiSt( \"include_test.mk\" )") << 1;
    QTest::newRow("file exists 3") << QByteArray("EXIST  ( include_test.mk\t)") << 1;
    QTest::newRow("file not exists") << QByteArray("EXIST(\"no such file\")") << 0;
    QTest::newRow("macro defined 1") << QByteArray("DEFINED ( ThisIsDefined\t)") << 1;
    QTest::newRow("macro defined 2") << QByteArray("DeFiNeD(\"ThisIsDefined\")") << 1;
    QTest::newRow("macro defined 3") << QByteArray("DEFINED\t(ThisIsDefinedButEmpty)") << 1;
    QTest::newRow("macro defined 4") << QByteArray("defined (   ThisIsUnfortunatelyNotDefined    )") << 0;
    QTest::newRow("shellcommand") << QByteArray("[ cmd /c exit 12 ]") << 12;
    QTest::newRow("ops +*") << QByteArray("2+3*5") << 17;
    QTest::newRow("ops (+)*") << QByteArray("(2+3)*5") << 25;
    QTest::newRow("op !") << QByteArray("!0") << 1;
    QTest::newRow("op !") << QByteArray("!(123)") << 0;
    QTest::newRow("op ~") << QByteArray("~(-1)") << 0;
    QTest::newRow("op ~") << QByteArray("~0") << -1;
    QTest::newRow("op -") << QByteArray("-156") << -156;
    QTest::newRow("op %") << QByteArray("156 % 7") << 2;
    QTest::newRow("op /") << QByteArray("156 / 12") << 13;
    QTest::newRow("op *") << QByteArray("12 * 13") << 156;
    QTest::newRow("op <<") << QByteArray("12 << 1") << 24;
    QTest::newRow("op >>") << QByteArray("12 >> 2") << 3;
    QTest::newRow("a > b") << QByteArray("1 > 0") << 1;
    QTest::newRow("a > b") << QByteArray("1 > 1") << 0;
    QTest::newRow("a >= b") << QByteArray("2 >= 1") << 1;
    QTest::newRow("a >= b") << QByteArray("1 >= 1") << 1;
    QTest::newRow("a >= b") << QByteArray("1 >= 2") << 0;
    QTest::newRow("a < b") << QByteArray("1 < 2") << 1;
    QTest::newRow("a < b") << QByteArray("1 < 1") << 0;
    QTest::newRow("a <= b") << QByteArray("1 <= 1") << 1;
    QTest::newRow("a <= b") << QByteArray("1 <= 2") << 1;
    QTest::newRow("a <= b") << QByteArray("2 <= 1") << 0;
    QTest::newRow("a == b") << QByteArray("156 == 156") << 1;
    QTest::newRow("a == b") << QByteArray("156 == 157") << 0;
    QTest::newRow("a != b") << QByteArray("156 != 156") << 0;
    QTest::newRow("a != b") << QByteArray("156 != 157") << 1;
    QTest::newRow("a && b") << QByteArray("156 && 157") << 1;
    QTest::newRow("a && b") << QByteArray("(156-156) && 157") << 0;
    QTest::newRow("a || b") << QByteArray("156 || 157") << 1;
    QTest::newRow("a || b") << QByteArray("(156-156) || 157") << 1;
    QTest::newRow("a || b") << QByteArray("(156-156) || 0") << 0;
    QTest::newRow("a & b") << QByteArray("156 & 12") << 12;
    QTest::newRow("a | b") << QByteArray("156 | 1") << 157;
    QTest::newRow("1+-1") << QByteArray("1+-1") << 0;
    QTest::newRow("string equality 1") << QByteArray("\"string one\" == \"string one\"") << 1;
    QTest::newRow("string equality 2") << QByteArray("\"one \"\" two\" == \"one \"\" two\"") << 1;
    QTest::newRow("string inquality") << QByteArray("\"one two\" != \"two one\"") << 1;
    QTest::newRow("string number mix") << QByteArray("(\"foo\" != \"bar\") == 1") << 1;
    QTest::newRow("macros in qmake Makefile") << QByteArray("\"$(QMAKESPEC)\" == \"win32-msvc\" || \"$(QMAKESPEC)\" == \"win32-msvc.net\" || \"$(QMAKESPEC)\" == \"win32-msvc2002\" || \"$(QMAKESPEC)\" == \"win32-msvc2003\" || \"$(QMAKESPEC)\" == \"win32-msvc2005\" || \"$(QMAKESPEC)\" == \"win32-msvc2008\" || \"$(QMAKESPEC)\" == \"win32-icc\"") << 1;
    QTest::newRow("pretty random test") << QByteArray("1 == 1 || 2 == 2 + 1 + 1 + -1|| 3 == 3") << 1;
}

void ParserTest::preprocessorExpressions()
{
    if (!m_preprocessor)
        m_preprocessor = new Preprocessor;

    MacroTable* macroTable = 0;
    QByteArray cdt(QTest::currentDataTag());
    if (cdt.startsWith("macro")) {
        macroTable = new MacroTable;
        macroTable->setMacroValue("ThisIsDefined", "yes");
        macroTable->setMacroValue("ThisIsDefinedButEmpty", QString());
        macroTable->setMacroValue("QMAKESPEC", "win32-msvc2008");
        m_preprocessor->setMacroTable(macroTable);
    }

    QFETCH(QByteArray, expression);
    QFETCH(int, expected);
    bool success = true;
    int expressionValue = -1;
    try {
        expressionValue = m_preprocessor->evaluateExpression(QString::fromLocal8Bit(expression.data()));
    } catch (...) {
        success = false;
    }
    QVERIFY(success);
    QCOMPARE(expressionValue, expected);

    if (macroTable) {
        m_preprocessor->setMacroTable(0);
        delete macroTable;
    }
}

void ParserTest::preprocessorDivideByZero()
{
    if (!m_preprocessor)
        m_preprocessor = new Preprocessor;

    NMakeFile::Exception error;
    bool exceptionCaught = false;
    try {
        m_preprocessor->evaluateExpression("1 / (156-156)");
    } catch (NMakeFile::Exception e) {
        exceptionCaught = true;
        error = e;
    }
    QCOMPARE(exceptionCaught, true);
    QVERIFY(error.message().contains("division by zero"));
}

void ParserTest::preprocessorInvalidExpressions_data()
{
     QTest::addColumn<QByteArray>("expression");
     QTest::newRow("empty") << QByteArray("");
     QTest::newRow("invalid characters") << QByteArray("\0x01\0x02\0x03");
     QTest::newRow("missing (") << QByteArray("1 + 1)");
     QTest::newRow("missing )") << QByteArray("(1 + 1");
     QTest::newRow("double op") << QByteArray("1++1");
     QTest::newRow("string == number") << QByteArray("\"foo\" == 156");
     //QTest::newRow("") << QByteArray("");
}

void ParserTest::preprocessorInvalidExpressions()
{
    if (!m_preprocessor)
        m_preprocessor = new Preprocessor;

    QFETCH(QByteArray, expression);
    NMakeFile::Exception error;
    bool exceptionCaught = false;
    try {
        m_preprocessor->evaluateExpression(QString::fromLocal8Bit(expression));
    } catch (NMakeFile::Exception e) {
        error = e;
        exceptionCaught = true;
    }

    QVERIFY(exceptionCaught);
    QVERIFY(!error.message().isEmpty());
}

void ParserTest::conditionals()
{
    if (!m_preprocessor)
        m_preprocessor = new Preprocessor;
    MacroTable* macroTable = new MacroTable;
    m_preprocessor->setMacroTable(macroTable);
    QVERIFY( m_preprocessor->openFile(QLatin1String("conditionals.mk")) );

    Parser parser;
    bool exceptionThrown = false;
    try {
        parser.apply(m_preprocessor);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    QCOMPARE(macroTable->macroValue("TEST1"), QLatin1String("true"));
    QCOMPARE(macroTable->macroValue("TEST2"), QLatin1String("true"));
    QCOMPARE(macroTable->macroValue("TEST3"), QLatin1String("true"));
    QCOMPARE(macroTable->macroValue("TEST4"), QLatin1String("true"));
    QCOMPARE(macroTable->macroValue("TEST5"), QLatin1String("true"));
    QCOMPARE(macroTable->macroValue("TEST6"), QLatin1String("true"));

    m_preprocessor->setMacroTable(0);
    delete macroTable;
}

void ParserTest::dotDirectives()
{
    if (!m_preprocessor)
        m_preprocessor = new Preprocessor;
    MacroTable* macroTable = new MacroTable;
    m_preprocessor->setMacroTable(macroTable);
    QVERIFY( m_preprocessor->openFile(QLatin1String("dotdirectives.mk")) );

    QSharedPointer<Makefile> mkfile;
    Parser parser;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(m_preprocessor);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    DescriptionBlock* target;
    Command cmd;

    target = mkfile->target(QLatin1String("silence_one"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.takeFirst();
    QCOMPARE(cmd.m_silent, false);

    target = mkfile->target(QLatin1String("silence_two"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.takeFirst();
    QCOMPARE(cmd.m_silent, true);

    target = mkfile->target(QLatin1String("silence_three"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.takeFirst();
    //QCOMPARE(cmd.m_silent, false);    // TODO: implement !CMDSWITCHES

    target = mkfile->target(QLatin1String("ignorance_one"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.takeFirst();
    QCOMPARE(int(cmd.m_maxExitCode), 0);

    target = mkfile->target(QLatin1String("ignorance_two"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 2);
    cmd = target->m_commands.takeFirst();
    QCOMPARE(int(cmd.m_maxExitCode), 255);

    target = mkfile->target(QLatin1String("ignorance_three"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.takeFirst();
    //QCOMPARE(int(cmd.m_maxExitCode), 0);   // TODO: implement !CMDSWITCHES

    QCOMPARE(mkfile->preciousTargets().count(), 3);
    QCOMPARE(mkfile->preciousTargets().at(0), QLatin1String("preciousness_one"));
    QCOMPARE(mkfile->preciousTargets().at(1), QLatin1String("preciousness_two"));
    QCOMPARE(mkfile->preciousTargets().at(2), QLatin1String("preciousness_three"));

    DescriptionBlock* suffixTarget = mkfile->target(QLatin1String("suffixes"));
    QVERIFY(suffixTarget != 0);
    QVERIFY(target->m_suffixes != suffixTarget->m_suffixes);

    delete macroTable;
}

void ParserTest::descriptionBlocks()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("descriptionBlocks.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    QVERIFY(mkfile != 0);
    DescriptionBlock* target = mkfile->target("one");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 3);
    QCOMPARE(target->m_dependents.at(0), QLatin1String("a"));
    QCOMPARE(target->m_dependents.at(1), QLatin1String("b"));
    QCOMPARE(target->m_dependents.at(2), QLatin1String("c"));
    QCOMPARE(target->m_commands.count(), 1);
    
    Command cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo one"));

    target = mkfile->target("two");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 0);
    QCOMPARE(target->m_commands.count(), 1);
    
    cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo two"));

    target = mkfile->target("three");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 1);
    QCOMPARE(target->m_commands.count(), 1);
    
    cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo three; @echo end of three"));

    target = mkfile->target(".");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo directory . doesn't exist. That's strange."));

    target = mkfile->target("..");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo directory .. doesn't exist. That's strange."));

    target = mkfile->target(QLatin1String("dollarSigns"));
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 0);
    QCOMPARE(target->m_commands.count(), 1);
    cmd = target->m_commands.first();
    QCOMPARE(cmd.m_commandLine, QLatin1String("echo ($dollar-signs$)"));
}

// inferenceRules test mode
static const char IRTM_Init = 0;
static const char IRTM_Cleanup = 1;
static const char IRTM_ParseTimeRule = 2;
static const char IRTM_DeferredRule = 3;

void ParserTest::inferenceRules_data()
{
    QTest::addColumn<char>("mode");
    QTest::addColumn<QString>("targetName");
    QTest::addColumn<QString>("expectedCommandLine");
    QTest::addColumn<QString>("fileToCreate");

    QTest::newRow("init") << IRTM_Init << "" << "" << "";
    QTest::newRow("1") << IRTM_ParseTimeRule << "foo1.obj" << "echo {subdir}.cpp.obj (subdir\\foo1.cpp)" << "";
    QTest::newRow("2") << IRTM_ParseTimeRule << "foo2.obj" << "echo {subdir}.cpp.obj (subdir\\foo2.cpp)" << "";
    QTest::newRow("3") << IRTM_ParseTimeRule << "foo3.obj" << "echo .cpp.obj (foo3.cpp)" << "";
    QTest::newRow("4") << IRTM_ParseTimeRule << "foo4.obj" << "echo {subdir}.cpp.obj (subdir\\foo4.cpp)" << "";
    QTest::newRow("5") << IRTM_DeferredRule << "foo5.obj" << "echo {subdir}.cpp.obj (subdir\\foo5.cpp)" << "subdir\\foo5.cpp";
    QTest::newRow("6") << IRTM_DeferredRule << "foo6.obj" << "echo .cpp.obj (foo6.cpp)" << "foo6.cpp";
    QTest::newRow("7") << IRTM_DeferredRule << "foo.bar.obj" << "echo .cpp.obj (foo.bar.cpp)" << "foo.bar.cpp";
    QTest::newRow("cleanup") << IRTM_Cleanup << "" << "" << "";

    QStringList filesToCreate;
    filesToCreate << "subdir\\foo5.cpp" << "foo6.cpp";
    foreach (const QString& fileName, filesToCreate)
        if (QFile::exists(fileName))
            system(qPrintable(QLatin1String("del ") + fileName));
}

void ParserTest::inferenceRules()
{
    static MacroTable* macroTable = 0;
    static Preprocessor* pp = 0;
    static Parser* parser = 0;
    static QSharedPointer<Makefile> mkfile;

    QFETCH(char, mode);
    QFETCH(QString, targetName);
    QFETCH(QString, expectedCommandLine);
    QFETCH(QString, fileToCreate);

    switch (mode) {
        case IRTM_Init: // init
            macroTable = new MacroTable;
            pp = new Preprocessor;
            parser = new Parser;
            QVERIFY(macroTable);
            QVERIFY(pp);
            QVERIFY(parser);
            pp->setMacroTable(macroTable);
            QVERIFY( pp->openFile(QLatin1String("infrules.mk")) );
            mkfile = parser->apply(pp);
            QVERIFY(mkfile);
            return;
        case IRTM_Cleanup: // cleanup
            delete parser;
            delete pp;
            delete macroTable;
            return;
    }

    DescriptionBlock* target = mkfile->target(targetName);
    mkfile->applyInferenceRules(target);
    QVERIFY(target);
    if (mode == IRTM_DeferredRule) {
        QVERIFY(target->m_commands.count() == 0);
        system(qPrintable(QLatin1String("echo.>") + fileToCreate));
        QVERIFY(QFile::exists(fileToCreate));
        mkfile->applyInferenceRules(target);
        system(qPrintable(QLatin1String("del ") + fileToCreate));
        QVERIFY(!QFile::exists(fileToCreate));
    }
    QVERIFY(target->m_commands.count() == 1);
    QCOMPARE(target->m_commands.first().m_commandLine, expectedCommandLine);
}

void ParserTest::cycleInTargets()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("cycle_in_targets.mk")) );

    bool exceptionThrown = false;
    try {
        parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(exceptionThrown);
}

void ParserTest::dependentsWithSpace()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("depswithspace.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (Exception e) {
        qDebug() << e.message();
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);
    QVERIFY(mkfile);
    DescriptionBlock* target = mkfile->target("first");
    QVERIFY(target);
    QCOMPARE(target->m_dependents.count(), 3);
    QCOMPARE(target->m_dependents.at(0), QLatin1String("one"));
    QCOMPARE(target->m_dependents.at(1), QLatin1String("\"dependent two with spaces\""));
    QCOMPARE(target->m_dependents.at(2), QLatin1String("three"));
}

void ParserTest::multipleTargets()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("targetmultidef.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (Exception e) {
        qDebug() << e.message();
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);
    QVERIFY(mkfile);
    DescriptionBlock* target = mkfile->target("foo");
    QVERIFY(target);
    QCOMPARE(target->m_dependents.count(), 3);
    QVERIFY(target->m_dependents.contains("foo1.cpp"));
    QVERIFY(target->m_dependents.contains("foo3.cpp"));
    QVERIFY(target->m_dependents.contains("foo4.cpp"));
    QCOMPARE(target->m_commands.count(), 1);

    target = mkfile->target("bar");
    QVERIFY(target);
    QCOMPARE(target->m_dependents.count(), 3);
    QVERIFY(target->m_dependents.contains("foo1.cpp"));
    QVERIFY(target->m_dependents.contains("foo3.cpp"));
    QVERIFY(target->m_dependents.contains("foo4.cpp"));
    QCOMPARE(target->m_commands.count(), 3);
}

void ParserTest::comments()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("comments.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);
    QCOMPARE(macroTable.macroValue("COMPILER"), QLatin1String("Ada95"));
    QCOMPARE(macroTable.macroValue("DEF"), QLatin1String("#define"));

    QVERIFY(mkfile);
    DescriptionBlock* target = mkfile->target("first");
    QVERIFY(target);
    QCOMPARE(target->m_dependents.count(), 2);
    QCOMPARE(target->m_commands.count(), 2);
    
    Command cmd1 = target->m_commands.at(0);
    Command cmd2 = target->m_commands.at(1);
    QCOMPARE(cmd1.m_commandLine, QLatin1String("echo I'm Winneone"));
    QCOMPARE(cmd2.m_commandLine, QLatin1String("echo I'm Winnetou"));

    target = mkfile->target("forth");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 1);
    QCOMPARE(target->m_commands.count(), 2);
    cmd1 = target->m_commands.at(0);
    cmd2 = target->m_commands.at(1);
    QCOMPARE(cmd1.m_commandLine, QLatin1String("echo # this is no comment"));
    QCOMPARE(cmd2.m_commandLine, QLatin1String("echo # this neither"));

    target = mkfile->target("fifth");
    QVERIFY(target != 0);
    QCOMPARE(target->m_dependents.count(), 1);
    QCOMPARE(target->m_dependents.first(), QLatin1String("file#99.txt"));
    QCOMPARE(target->m_commands.count(), 1);
}

void ParserTest::fileNameMacros()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    macroTable.setMacroValue("MAKEDIR", QDir::currentPath());
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("filenamemacros.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    DescriptionBlock* target;
    Command command;
    target = mkfile->target(QLatin1String("all"));
    QVERIFY(target);
    QVERIFY(target->m_dependents.contains("Football"));

    target = mkfile->target(QLatin1String("Football"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QVERIFY(!target->m_commands.isEmpty());
    command = target->m_commands.first();
    QCOMPARE(command.m_commandLine, QLatin1String("echo Football Football"));

    target = mkfile->target(QLatin1String("LolCatExtractorManager.tar.gz"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QVERIFY(!target->m_commands.isEmpty());
    command = target->m_commands.first();
    QCOMPARE(command.m_commandLine, QLatin1String("echo LolCatExtractorManager.tar"));

    target = mkfile->target(QLatin1String("manyDependents"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QVERIFY(!target->m_commands.isEmpty());
    command = target->m_commands.takeFirst();
    QCOMPARE(command.m_commandLine, QLatin1String("echo Timmy Jimmy Kenny Eric Kyle Stan"));
    QVERIFY(!target->m_commands.isEmpty());
    command = target->m_commands.first();
    QCOMPARE(command.m_commandLine, QLatin1String("echo Timmy Jimmy Kenny Eric Kyle Stan"));

    target = mkfile->target(QLatin1String("manyDependentsSingleExecution"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QCOMPARE(target->m_commands.size(), 12);
    QStringList lst = QStringList() << "Timmy" << "Jimmy" << "Kenny" << "Eric" << "Kyle" << "Stan";
    lst.append(lst);
    foreach (const QString& str, lst) {
        QVERIFY(!target->m_commands.isEmpty());
        command = target->m_commands.takeFirst();
        QCOMPARE(command.m_commandLine, QLatin1String("echo ") + str);
    }

    system("del generated.txt gen1.txt gen2.txt gen3.txt > NUL 2>&1");
    target = mkfile->target(QLatin1String("gen_init"));
    QVERIFY(target);
    QVERIFY(!target->m_commands.isEmpty());
    foreach (const Command& cmd, target->m_commands)
        system(qPrintable(cmd.m_commandLine));

    target = mkfile->target(QLatin1String("generated.txt"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QVERIFY(!target->m_commands.isEmpty());
    command = target->m_commands.first();
    QCOMPARE(command.m_commandLine, QLatin1String("echo gen2.txt gen3.txt"));

    target = mkfile->target(QLatin1String("gen_cleanup"));
    QVERIFY(target);
    QVERIFY(!target->m_commands.isEmpty());
    foreach (const Command& cmd, target->m_commands)
        system(qPrintable(cmd.m_commandLine));

    target = mkfile->target(QLatin1String("macros.mk"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QCOMPARE(target->m_commands.count(), 4);
    command = target->m_commands.at(0);
    QCOMPARE(command.m_commandLine, QLatin1String("echo ."));
    command = target->m_commands.at(1);
    QCOMPARE(command.m_commandLine, QLatin1String("echo macros"));
    command = target->m_commands.at(2);
    QCOMPARE(command.m_commandLine, QLatin1String("echo macros.mk"));
    command = target->m_commands.at(3);
    QCOMPARE(command.m_commandLine, QLatin1String("echo macros"));

    const QString currentPath = QDir::currentPath().replace('/', '\\');
    target = mkfile->target(currentPath + QLatin1String("\\infrules.mk"));
    QVERIFY(target);
    target->expandFileNameMacros();
    QCOMPARE(target->m_commands.count(), 4);
    command = target->m_commands.at(0);
    QCOMPARE(command.m_commandLine, QLatin1String("echo ") + currentPath);
    command = target->m_commands.at(1);
    QCOMPARE(command.m_commandLine, QLatin1String("echo infrules"));
    command = target->m_commands.at(2);
    QCOMPARE(command.m_commandLine, QLatin1String("echo infrules.mk"));
    command = target->m_commands.at(3);
    QCOMPARE(command.m_commandLine, QLatin1String("echo ") + currentPath + QLatin1String("\\infrules"));
}

void ParserTest::fileNameMacrosInDependents()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("fileNameMacrosInDependents.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    DescriptionBlock* target;
    target = mkfile->target(QLatin1String("foo"));
    QVERIFY(target);
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $*
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $$(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $(*D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*R)

    target = mkfile->target(QLatin1String("foo.obj"));
    QVERIFY(target);
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $$@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $*
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $$(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("."));                                 // $(*D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $$(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*R)

    target = mkfile->target(QLatin1String("C:\\MyProject\\tmp\\foo.obj"));
    QVERIFY(target);
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo.obj"));       // $@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo.obj"));       // $$@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo"));           // $*
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp"));                // $(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp"));                // $$(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp"));                // $(*D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $$(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo"));           // $(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo"));           // $$(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("C:\\MyProject\\tmp\\foo"));           // $(*R)

    target = mkfile->target(QLatin1String("\"C:\\My Project\\tmp\\foo.obj\""));
    QVERIFY(target);
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo.obj\""));  // $@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo.obj\""));  // $$@
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo\""));      // $*
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\""));           // $(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\""));           // $$(@D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\""));           // $(*D)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $$(@B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*B)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo.obj"));                           // $$(@F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("foo"));                               // $(*F)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo\""));      // $(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo\""));      // $$(@R)
    QCOMPARE(target->m_dependents.takeFirst(), QLatin1String("\"C:\\My Project\\tmp\\foo\""));      // $(*R)
}

void ParserTest::windowsPathsInTargetName()
{
    MacroTable macroTable;
    Preprocessor pp;
    Parser parser;
    macroTable.setMacroValue("MAKEDIR", QDir::currentPath());
    pp.setMacroTable(&macroTable);
    QVERIFY( pp.openFile(QLatin1String("windowspaths.mk")) );

    QSharedPointer<Makefile> mkfile;
    bool exceptionThrown = false;
    try {
        mkfile = parser.apply(&pp);
    } catch (...) {
        exceptionThrown = true;
    }
    QVERIFY(!exceptionThrown);

    DescriptionBlock* target = mkfile->firstTarget();
    QVERIFY(target != 0);
    QCOMPARE(target->targetName(), QLatin1String("C:\\foo.txt"));

    target = mkfile->target(QLatin1String("C:\\bar.txt"));
    QVERIFY(target != 0);
    QCOMPARE(target->targetName(), QLatin1String("C:\\bar.txt"));

    target = mkfile->target(QLatin1String("\"C:\\three.txt\""));
    QVERIFY(target != 0);
    QCOMPARE(target->targetName(), QLatin1String("\"C:\\three.txt\""));
    QCOMPARE(target->m_commands.count(), 2);

    target = mkfile->target(QLatin1String("S"));
    QVERIFY(target != 0);
    QCOMPARE(target->targetName(), QLatin1String("S"));
    QCOMPARE(target->m_commands.count(), 2);
}

QTEST_MAIN(ParserTest)
#include "tests.moc"
