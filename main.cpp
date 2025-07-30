#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <deque>
#include <QPixmap>
#include <QVector>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QFont>
#include <QFontDatabase>
#include <QFileDialog>
#include <QJsonArray>
#include <QTabWidget>
#include <QPropertyAnimation>
#include <QProcess>
#include <QListWidget>
#include <future>
#include <thread>
#include <QThread>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#define CPPHTTPLIB_OPENSSL_SUPPORT // f**k you
#include <openssl/ssl.h> // f**k you
#include "httplib.h" // and f**k you
#include <QDesktopServices>
#include <QFileSystemWatcher>

#ifdef max // what is this  - alice
#undef max
#endif

// TODO
// make buttons in tab for opening a mini text editor for the style and config and atlas.json
// we should probably get more devs for this
// a credits tab

#define APP_VERSION 4

enum class Rank { Small = 0, Medium = 1, Large = 2 };

enum class Variant { A = 0, B = 1 };

struct Particle {
    QPointF pos;
    QPointF velocity;
    float age = 0.0f;
    Rank rank;
    Variant variant;
};

struct SpriteFrame {
    QRect rect;
    QPointF offset;
};

struct ParticleConfig {
    float lifetime = 1.0f;
    float lifeDelta = 1.0f;
    size_t maxParticles = 1000;

    size_t spawnLimit = 100;
    float spawnSpeed = 10.0f;
    float spawnWind = 100.0f;
    float spawnVX = 0.0f;
    float spawnVY = 0.0f;

    float friction = 0.9f;
    float jitter = 25.0f;
    float wind = 10.0f;
    float bounce = 1.0f;
    float gravityX = 0.0f;
    float gravityY = 10.0f;

    float animSpeed = 10.0f;
    float shineSpeed = 60.0f;
    float shinePower = 0.5f;
    float opacity = 1.0f;
    float scale = 2.5f;

    float whiteOutline = 0.2f;
    float blackOutline = 0.1f;
    float red = 1.0f;
    float green = 0.6f;
    float blue = 0.1f;

    QJsonObject toJson() const {
        QJsonObject o;
        o["lifetime"] = lifetime;
        o["lifeDelta"] = lifeDelta;
        o["maxParticles"] = static_cast<int>(maxParticles);

        o["spawnLimit"] = static_cast<int>(spawnLimit);
        o["spawnSpeed"] = spawnSpeed;
        o["spawnWind"] = spawnWind;
        o["spawnVX"] = spawnVX;
        o["spawnVY"] = spawnVY;

        o["friction"] = friction;
        o["jitter"] = jitter;
        o["wind"] = wind;
        o["bounce"] = bounce;
        o["gravityX"] = gravityX;
        o["gravityY"] = gravityY;

        o["animSpeed"] = animSpeed;
        o["shineSpeed"] = shineSpeed;
        o["shinePower"] = shinePower;
        o["opacity"] = opacity;
        o["scale"] = scale;

        o["whiteOutline"] = whiteOutline;
        o["blackOutline"] = blackOutline;
        o["red"] = red;
        o["green"] = green;
        o["blue"] = blue;
        return o;
    }

    static int getIntOrDefault(const QJsonObject &o, const QString &key, int def) {
        return o.contains(key) && o[key].isDouble() ? o[key].toInt(def) : def;
    }

    static size_t getSizeTOrDefault(const QJsonObject &o, const QString &key, size_t def) {
        return static_cast<size_t>(getIntOrDefault(o, key, static_cast<int>(def)));
    }

    static float getFloatOrDefault(const QJsonObject &o, const QString &key, float def) {
        return o.contains(key) && o[key].isDouble() ? static_cast<float>(o[key].toDouble(def)) : def;
    }

    void fromJson(const QJsonObject &o) {
        lifetime = getFloatOrDefault(o, "lifetime", lifetime);
        lifeDelta = getFloatOrDefault(o, "lifeDelta", lifeDelta);
        maxParticles = getSizeTOrDefault(o, "maxParticles", maxParticles);

        spawnLimit = getSizeTOrDefault(o, "spawnLimit", spawnLimit);
        spawnSpeed = getFloatOrDefault(o, "spawnSpeed", spawnSpeed);
        spawnWind = getFloatOrDefault(o, "spawnWind", spawnWind);
        spawnVX = getFloatOrDefault(o, "spawnVX", spawnVX);
        spawnVY = getFloatOrDefault(o, "spawnVY", spawnVY);

        friction = getFloatOrDefault(o, "friction", friction);
        jitter = getFloatOrDefault(o, "jitter", jitter);
        wind = getFloatOrDefault(o, "wind", wind);
        bounce = getFloatOrDefault(o, "bounce", bounce);
        gravityX = getFloatOrDefault(o, "gravityX", gravityX);
        gravityY = getFloatOrDefault(o, "gravityY", gravityY);

        animSpeed = getFloatOrDefault(o, "animSpeed", animSpeed);
        shineSpeed = getFloatOrDefault(o, "shineSpeed", shineSpeed);
        shinePower = getFloatOrDefault(o, "shinePower", shinePower);
        scale = getFloatOrDefault(o, "scale", scale);

        whiteOutline = getFloatOrDefault(o, "whiteOutline", whiteOutline);
        blackOutline = getFloatOrDefault(o, "blackOutline", blackOutline);
        red = getFloatOrDefault(o, "red", red);
        green = getFloatOrDefault(o, "green", green);
        blue = getFloatOrDefault(o, "blue", blue);
    }
};

    void saveToFile(const std::string& filename, const std::string& content) {
        std::ofstream out(filename);
        if (out) {
            out << content;
            out.close();
        } else {
            std::cerr << "Failed to open file: " << filename << std::endl;
        }
    }

class CleanSpinBox : public QDoubleSpinBox {
    Q_OBJECT

public:
    enum class Mode {
        Float,
        Integer
    };

    CleanSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent), mode(Mode::Float) {
        setDecimals(3);
    }

    void setMode(Mode m) {
        mode = m;
        if (mode == Mode::Integer) {
            setDecimals(0);
        }
    }

    Mode getMode() const { return mode; }

protected:
    QString textFromValue(double value) const override {
        if (mode == Mode::Integer) {
            return QString::number(static_cast<int>(value));
        } else {
            return QString::number(value, 'g', 5);
        }
    }

    double valueFromText(const QString &text) const override {
        return text.toDouble();
    }

private:
    Mode mode;
};

class CPPoorklMenuWidget : public QWidget {
    Q_OBJECT

public:
    CPPoorklMenuWidget(QWidget *parent = nullptr)
        : QWidget(parent) {
        tabs = new QTabWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(tabs);
        setLayout(layout);
        setWindowFlags(Qt::FramelessWindowHint);
        // resize(200, 100);
        show();

        connect(tabs, &QTabWidget::currentChanged, this, [this](int idx){
            emit tabChanged(idx, tabs->tabText(idx));
        });
    }

    void addButton(const QString &tabName, const QString &name, std::function<void()> onClick) {
        auto tabLayout = tabLayouts[tabName];
        if (!tabLayout) return;
        QPushButton *btn = new QPushButton(name);
        connect(btn, &QPushButton::clicked, this, [=]() { onClick(); });
        tabLayout->addWidget(btn);
    }

    void addTab(const QString &name) {
        QWidget *tab = new QWidget();
        QVBoxLayout *tabLayout = new QVBoxLayout(tab);
        tab->setLayout(tabLayout);
        tabs->addTab(tab, name);
        tabContents[name] = tab;
        tabLayouts[name] = tabLayout;
    }


    void addFloatControl(const QString &tabName, const QString &name, float min, float max, float step,
                         float &targetVariable) {
        auto tabLayout = tabLayouts[tabName];
        if (!tabLayout) return;
        QWidget *container = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(container);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel(name);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLayout->addWidget(label);

        CleanSpinBox *spin = new CleanSpinBox();
        spin->setMode(CleanSpinBox::Mode::Float);
        spin->setRange(min, max);
        spin->setSingleStep(step);
        spin->setValue(targetVariable);
        rowLayout->addWidget(spin);

        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [&, label](double val) {
                    targetVariable = static_cast<float>(val);
                    emit parameterChanged();
                });

        spinboxes.append(spin);
        tabLayout->addWidget(container);
    }

    void addIntControl(const QString &tabName, const QString &name, int min, int max, size_t &targetVariable) {
        auto tabLayout = tabLayouts[tabName];
        if (!tabLayout) return;
        QWidget *container = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(container);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel(name);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rowLayout->addWidget(label);

        CleanSpinBox *spin = new CleanSpinBox();
        spin->setMode(CleanSpinBox::Mode::Integer);
        spin->setRange(min, max);
        spin->setSingleStep(1);
        spin->setValue(static_cast<int>(targetVariable));
        rowLayout->addWidget(spin);

        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [&, this](double val) {
                    targetVariable = static_cast<size_t>(val);
                    emit parameterChanged();
                });

        spinboxes.append(spin);
        tabLayout->addWidget(container);
    }

    void updateSpinboxValues(const QVector<float> &values) {
        for (int i = 0; i < spinboxes.size() && i < values.size(); ++i) {
            spinboxes[i]->blockSignals(true);
            spinboxes[i]->setValue(values[i]);
            spinboxes[i]->blockSignals(false);
        }
    }

    void addWidgetToTab(const QString &tabName, QWidget *widget) {
        auto tabLayout = tabLayouts.value(tabName, nullptr);
        if (tabLayout) {
            tabLayout->addWidget(widget);
        }
    }

signals:
    void parameterChanged();
    void tabChanged(int index, const QString &name);

protected:
    QPoint dragStartPos;
    bool dragging = false;
    QTabWidget *tabs;
    QMap<QString, QWidget *> tabContents;
    QMap<QString, QVBoxLayout *> tabLayouts;
    QVector<QDoubleSpinBox *> spinboxes;

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
            dragging = true;
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - dragStartPos);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            dragging = false;
            event->accept();
        }
    }
};
bool checkOK = true;

class ParticleWidget : public QWidget {
    Q_OBJECT

public:
    void updatePresetList() {
        fileList->clear();
        QDir dir("presets");
        QStringList files = dir.entryList(QDir::Files);
        QMap<QString, QSet<QString>> groups;

        for (const QString& file : files) { // cpp11 range-loop might detach qt container (qstringlist)  - alice
            QString prefix;
            if (file.endsWith("-atlas.png"))
                prefix = file.left(file.length() - 10);
            else if (file.endsWith("-atlas.json"))
                prefix = file.left(file.length() - 11);
            else if (file.endsWith("-config.json"))
                prefix = file.left(file.length() - 12);
            else
                continue;

            groups[prefix].insert(file.section('-', -1));
        }

        for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
            const QSet<QString>& tags = it.value();

            bool hasPng = tags.contains("atlas.png");
            bool hasJson = tags.contains("atlas.json");
            bool hasConfig = tags.contains("config.json");

            if (hasPng && hasJson && hasConfig) {
                fileList->addItem(it.key());
            } else {
                fileList->addItem("! " + it.key());
            }
        }
    }
    ParticleWidget(QWidget *parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setFocusPolicy(Qt::StrongFocus);

        loadConfig(QCoreApplication::applicationDirPath() + "/config.json", false);
        loadSprites(QCoreApplication::applicationDirPath() + "/atlas.png", QCoreApplication::applicationDirPath() + "/atlas.json");
        loadStyle(QCoreApplication::applicationDirPath() + "/style.qss");
        loadFont(QCoreApplication::applicationDirPath() + "/font.ttf");
        menu = new CPPoorklMenuWidget();
        //THIS IS WHERE THE MAGIC HAPPENS
        bool canupdater = true;
        //autoupdate code start
        httplib::Client cliUpd("https://raw.githubusercontent.com");
        std::string updPath = "/la-creatura/spoorkl/main/version";
        auto res = cliUpd.Get(updPath);
        if (res && res->status == 200) {
            int remoteVersion = std::stoi(std::regex_replace(res->body, std::regex("\\n"), ""));
            std::string remoteVersionS = std::regex_replace(res->body, std::regex("\\n"), "");
            if (remoteVersion > APP_VERSION) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "update",
                    "new version available! do you want to download it?",
                    QMessageBox::Yes | QMessageBox::No
                );
                /*std::string filename = "updater.bat";
                std::string content =
                    "@echo off\n"
                    "setlocal\n"
                    "timeout /t 3 >nul\n"
                    "echo searching for updated file\n"
                    "set \"FILENAME=CPPOORKL_v"+remoteVersionS+".exe\"\n"
                    "for /l %%i in (1,1,5) do (\n"
                    "    if exist \"%%FILENAME%%\" (\n"
                    "        start \"\" \"%%FILENAME%%\"\n"
                    "        goto END\n"
                    "    )\n"
                    "    timeout /t 1 >nul\n"
                    ")\n"
                    "echo updated file not found, try running CPPOORKL_v"+remoteVersionS+".exe manually\n"
                    ":END\n"
                    "endlocal\n";
                saveToFile(filename, content);*/
                if (reply == QMessageBox::Yes) {
                    std::string path2 = "/la-creatura/spoorkl/main/latest.exe";
                    auto res = cliUpd.Get(path2);
                    if (res && res->status == 200) {
                        QFile sfile(QString::fromStdString("CPPOORKL_v" + remoteVersionS + ".exe"));
                        if (sfile.open(QIODevice::WriteOnly)) {
                            sfile.write(res->body.c_str(), static_cast<qint64>(res->body.size()));
                            sfile.close();
                        }
                    } else {
                        QString message = "could not download updated file";
                        QMessageBox::critical(this, "update", message);
                        canupdater = false;
                    }
                    if (canupdater) {
                        std::string exeName = "CPPOORKL_v" + remoteVersionS + ".exe";
                        WinExec(exeName.c_str(), SW_SHOW);
                        QThread::sleep(1);
                        ExitProcess(0);
                    }
                }
            }
        } else {
            QMessageBox::warning(this, "update", "could not connect to update server");
        }
        //autoupdate code end

        //start preset browser setup
        browserList = new QListWidget();
        browserList->addItem("loading...");

        connect(browserList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
            if (item->text() != "hm, cant connect") {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "download?",
                    "do you want to download " + item->text() + "?",
                    QMessageBox::Yes | QMessageBox::No
                );

                if (reply == QMessageBox::Yes) {
                    checkOK = true;
                    httplib::Client cli("https://raw.githubusercontent.com");
                    std::vector<std::string> remoteFiles = {
                        "atlas.json",
                        "config.json"
                    };

                    for (const auto& remoteFile : remoteFiles) {
                        std::string path = "/AACCBB80/CPPoorkl_presets/main/" + item->text().toStdString() + "/" + remoteFile;
                        auto res = cli.Get(path);
                        if (res && res->status == 200) {
                            saveToFile("presets/"+item->text().toStdString() + "-" + remoteFile, res->body);
                        } else {

                            QString message = "can't download \"" + item->text() + "/" + QString::fromStdString(remoteFile) + "\"";
                            QMessageBox::critical(this, "error", message);
                            checkOK = false;
                        }
                    }
                    std::string path = "/AACCBB80/CPPoorkl_presets/main/" + item->text().toStdString() + "/atlas.png";
                    auto res = cli.Get(path);
                    if (res && res->status == 200) {
                        QFile file("presets/"+item->text() + "-" + "atlas.png");
                        if (file.open(QIODevice::WriteOnly)) {
                            file.write(res->body.c_str(), static_cast<qint64>(res->body.size()));
                            file.close();
                        }
                    } else {
                        QString message = "can't download \"" + item->text() + "/atlas.png\"";
                        QMessageBox::critical(this, "error", message);
                        checkOK = false;
                    }
                    if (checkOK==true) {
                        loadConfig("presets/"+item->text() + "-config.json", true);
                        loadSprites("presets/"+item->text() + "-atlas.png", item->text() + "-atlas.json");
                    } else {
                        QMessageBox::critical(this, "error", "aborted automatically applying preset due to download errors");
                    }
                }
            }
        });

        fileList = new QListWidget();
        connect(fileList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
            if (!(item->text().startsWith("!"))) {
                loadConfig("presets/"+item->text() + "-config.json", true);
                loadSprites("presets/"+item->text() + "-atlas.png", "presets/"+item->text() + "-atlas.json");
            }
        });

        QString folderPath = QCoreApplication::applicationDirPath() + "/presets";
        QDir dir(folderPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }


        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ParticleWidget::onTimeout);
        timer->start(16);

        setMouseTracking(true);
        menu->addTab("config");
        menu->addTab("spawn");
        menu->addTab("physics");
        menu->addTab("visual");
        menu->addTab("atlas");
        menu->addTab("online"); //aaccbb80  goog job  - alice
        menu->addTab("presets"); //aaccbb80
        menu->addTab("import");
        menu->addTab("export");
        menu->addTab("debug");

        menu->addFloatControl("config", "lifetime", -1e9f, 1e9f, 0.01f, config.lifetime);
        menu->addFloatControl("config", "lifeDelta", -1e9f, 1e9f, 0.01f, config.lifeDelta);
        menu->addIntControl("config", "maxParticles", 0, 1e9f, config.maxParticles);

        menu->addIntControl("spawn", "spawnLimit", 0, 1e9f, config.spawnLimit);
        menu->addFloatControl("spawn", "spawnSpeed", -1e9f, 1e9f, 0.01f, config.spawnSpeed);
        menu->addFloatControl("spawn", "spawnWind", -1e9f, 1e9f, 0.01f, config.spawnWind);
        menu->addFloatControl("spawn", "spawnVX", -1e9f, 1e9f, 0.01f, config.spawnVX);
        menu->addFloatControl("spawn", "spawnVY", -1e9f, 1e9f, 0.01f, config.spawnVY);

        menu->addFloatControl("physics", "friction", 0.0f, 1.0f, 0.01f, config.friction);
        menu->addFloatControl("physics", "jitter", -1e9f, 1e9f, 0.01f, config.jitter);
        menu->addFloatControl("physics", "wind", -1e9f, 1e9f, 0.01f, config.wind);
        menu->addFloatControl("physics", "bounce", -1.0f, 1.0f, 0.01f, config.bounce);
        menu->addFloatControl("physics", "gravityX", -1e9f, 1e9f, 0.01f, config.gravityX);
        menu->addFloatControl("physics", "gravityY", -1e9f, 1e9f, 0.01f, config.gravityY);

        menu->addFloatControl("visual", "animSpeed", -1e9f, 1e9f, 0.01f, config.animSpeed);
        menu->addFloatControl("visual", "shineSpeed", -1e9f, 1e9f, 0.01f, config.shineSpeed);
        menu->addFloatControl("visual", "shinePower", -1e9f, 1e9f, 0.01f, config.shinePower);
        menu->addFloatControl("visual", "opacity", 0.0f, 1.0f, 0.01f, config.opacity);
        menu->addFloatControl("visual", "scale", -1e9f, 1e9f, 0.01f, config.scale);

        menu->addFloatControl("atlas", "whiteOutline", 0.0f, 1.0f, 0.01f, config.whiteOutline);
        menu->addFloatControl("atlas", "blackOutline", 0.0f, 1.0f, 0.01f, config.blackOutline);
        menu->addFloatControl("atlas", "red", 0.0f, 1.0f, 0.01f, config.red);
        menu->addFloatControl("atlas", "green", 0.0f, 1.0f, 0.01f, config.green);
        menu->addFloatControl("atlas", "blue", 0.0f, 1.0f, 0.01f, config.blue);

        QLabel* linklabel = new QLabel(QString( // absolute cinema  - alice
            "<table width='100%' style='font-size:8pt;'>"
            "<tr>"
            "<td align='left'>select preset to download</td>"
            "<td align='right'><a href='https://github.com/AACCBB80/CPPoorkl_presets/blob/main/README.md'>upload</a></td>"
            "</tr>"
            "</table>"
        ));
        linklabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        linklabel->setOpenExternalLinks(true);

        menu->addWidgetToTab("online", linklabel);
        menu->addWidgetToTab("online", browserList);
        menu->addWidgetToTab("online", new QLabel(QString("<span style='font-size:5pt'>preset browser programmed by aaccbb80</span>")));

        QLabel* linklabel2 = new QLabel(QString( // absolute cinema 2  - alice
            "<table width='100%' style='font-size:8pt;'>"
            "<tr>"
            "<td align='left'>select preset to apply</td>"
            "<td align='right'><a href='#'>open folder</a></td>"
            "</tr>"
            "</table>"
        ));
        linklabel2->setTextInteractionFlags(Qt::TextBrowserInteraction);
        linklabel2->setOpenExternalLinks(false);
        connect(linklabel2, &QLabel::linkActivated, this, [=](const QString &) {
            QString exePath = QCoreApplication::applicationDirPath();
            QDesktopServices::openUrl(QUrl("file:///" + exePath + "/presets/"));
        });

        menu->addWidgetToTab("presets", linklabel2);
        menu->addWidgetToTab("presets", fileList);
        menu->addWidgetToTab("presets", new QLabel(QString("<span style='font-size:5pt'>preset list programmed by aaccbb80</span>"))); // acb what's w/ you using html for this ;w;  - alice


        menu->addButton("import", "import config", [this]() { importConfig(); });
        menu->addButton("import", "import style", [this]() { importStyle(); });
        menu->addButton("import", "import font", [this]() { importFont(); });
        menu->addButton("import", "import atlas", [this]() { importSprites(); });

        menu->addButton("export", "export config", [this]() { exportConfig(); });
        menu->addButton("export", "export style", [this]() { exportStyle(); });
        menu->addButton("export", "export font", [this]() { exportFont(); });
        menu->addButton("export", "export atlas", [this]() { exportSprites(); });


        menu->addButton("debug", "restart", [this]() { restartApp(); });

        fpsLabel = new QLabel("FPS: 0");
        particleCountLabel = new QLabel("Particles: 0");

        menu->addWidgetToTab("debug", fpsLabel);
        menu->addWidgetToTab("debug", particleCountLabel);

        frameTimer.start();

        updatePresetList();

        watcher = new QFileSystemWatcher(this);
        watcher->addPath("presets");
        connect(watcher, &QFileSystemWatcher::directoryChanged,
                this, &ParticleWidget::updatePresetList);

        connect(menu, &CPPoorklMenuWidget::tabChanged, this, [this](int, const QString &name) {
            if (name != "online") return;

            // auto *worker = new QObject;
            QThread *thread = QThread::create([this]() {
                httplib::Client cli("https://raw.githubusercontent.com"); //ИДИ НА**Й

                auto res = cli.Get("/AACCBB80/CPPoorkl_presets/refs/heads/main/list"); //im going to f***ing k*** myself
                QString result; // я совираюсь т**хнуть себя в РОТ :heart_on_fire:

                if (res && res->status == 200)
                    result = QString::fromStdString(res->body); // ще се из***ам в устата UwU

                // this wasn't a UI issue, oops.
                QMetaObject::invokeMethod(QApplication::instance(), [this, result]() {
                    if (!browserList) return;
                    browserList->clear();
                    if (result.isEmpty()) { // RESULT IS NOT F**KING EMPTY (yes it was)
                        browserList->addItems({
                            "hm, cant connect",
                        });
                    } else {
                        const auto lines = result.split('\n', Qt::SkipEmptyParts);
                        for (const QString &line : lines)
                            browserList->addItem(line.trimmed());
                    }
                });
            });

            // this has been fixed; doesn't cause memory leaks anymore
            connect(thread, &QThread::finished, thread, &QObject::deleteLater);
            thread->start();

        });
    }

    void exportConfig() {
        QString path = QFileDialog::getSaveFileName(this, "export config", "", "json files (*.json)");
        if (path.isEmpty()) return;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(config.toJson()).toJson(QJsonDocument::Indented));
            QMessageBox::information(this, "export", "configuration exported successfully");
        } else {
            QMessageBox::warning(this, "export Failed", "could not write config");
        }
    }

    void importConfig() {
        QString path = QFileDialog::getOpenFileName(this, "import config", "", "json files (*.json)");
        if (path.isEmpty()) return;
        loadConfig(path, true);
    }

    void loadConfig(const QString &path, const bool updateSpinboxes) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) return;
        config.fromJson(doc.object());
        if (updateSpinboxes) {
            QVector<float> values = {
                config.lifetime,
                config.lifeDelta,
                static_cast<float>(config.maxParticles),

                static_cast<float>(config.spawnLimit),
                config.spawnSpeed,
                config.spawnWind,
                config.spawnVX,
                config.spawnVY,

                config.friction,
                config.jitter,
                config.wind,
                config.bounce,
                config.gravityX,
                config.gravityY,

                config.animSpeed,
                config.shineSpeed,
                config.shinePower,
                config.opacity,
                config.scale,

                config.whiteOutline,
                config.blackOutline,
                config.red,
                config.green,
                config.blue
            };
            menu->updateSpinboxValues(values);
        }
    }

    void exportStyle() {
        QString path = QFileDialog::getSaveFileName(this, "export style", "", "qss files (*.qss)");
        if (!path.isEmpty()) {
            QFile source(QCoreApplication::applicationDirPath() + "/style.qss");
            if (source.open(QIODevice::ReadOnly)) {
                QFile dest(path);
                if (dest.open(QIODevice::WriteOnly)) {
                    dest.write(source.readAll());
                }
            }
        }
    }

    void importStyle() {
        QString path = QFileDialog::getOpenFileName(this, "import style", "", "qss files (*.qss)");
        if (path.isEmpty()) return;
        loadStyle(path);
    }

    void loadStyle(const QString &path) {
        QFile file(path);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString style = file.readAll();
            QApplication *app = qobject_cast<QApplication *>(QApplication::instance());
            app->setStyleSheet(style);
        } else {
            QMessageBox::warning(this, "style load error", "failed to load style: " + path);
        }
    }

    void exportFont() {
        QMessageBox::warning(this, "not implemented error", "alice too lazy to implement this export font button");
    }

    void importFont() {
        QString path = QFileDialog::getOpenFileName(this, "import font", "", "truetypeface files (*.ttf)");
        if (path.isEmpty()) return;
        loadFont(path);
    }

    void loadFont(const QString &path) {
        int fontId = QFontDatabase::addApplicationFont(path);
        if (fontId != -1) {
            QStringList families = QFontDatabase::applicationFontFamilies(fontId);
            if (!families.isEmpty()) {
                QFont font(families.first());
                QApplication::setFont(font);
            }
        }
    }

    void exportSprites() {
        QMessageBox::warning(this, "not implemented error", "alice too lazy to implement this export atlas button");
    }

    void importSprites() {
        QString imagePath = QFileDialog::getOpenFileName(this, "import atlas image", "", "image files (*.png *.jpg)");
        if (imagePath.isEmpty())
            return;

        QString jsonPath = QFileDialog::getOpenFileName(this, "import atlas json", "", "json files (*.json)");
        if (jsonPath.isEmpty())
            return;

        loadSprites(imagePath, jsonPath);
    }

    void loadSprites(const QString &atlasPath, const QString &jsonPath) {
        QImage image;
        if (!image.load(atlasPath)) {
            QMessageBox::warning(this, "atlas load error", "failed to load atlas image: " + atlasPath);
        }
        QImage shifted = applyColourTransform(image, config.red, config.green, config.blue);
        atlas = QPixmap::fromImage(shifted);

        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "atlas load error", "failed to load atlas json: " + jsonPath);
            return;
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, "json parse error", "failed to parse atlas json: " + err.errorString());
            return;
        }

        QJsonObject root = doc.object();
        for (auto it = root.begin(); it != root.end(); ++it) {
            QString name = it.key();
            QJsonObject obj = it.value().toObject();

            QRect rect(obj["x"].toInt(), obj["y"].toInt(), obj["w"].toInt(), obj["h"].toInt());
            QPointF offset(obj["ox"].toDouble(), obj["oy"].toDouble());

            spriteRects[name] = SpriteFrame{rect, offset};
        }
    }

    void restartApp() {
        QString executable = QCoreApplication::applicationFilePath();
        QStringList args = QCoreApplication::arguments();

        QProcess::startDetached(executable, args);
        QCoreApplication::quit();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setRenderHint(QPainter::SmoothPixmapTransform, false);
        p.setCompositionMode(QPainter::CompositionMode_Plus); // PoS composition mode doesn't make glowy 3:<  - alice
        p.setOpacity(config.opacity);

        for (const auto &particle: particles) {
            float ageRatio = qBound(0.0f, particle.age / config.lifetime, 1.0f); // this should always work with negative life   - alice
            float invAgeRatio = 1.0f - ageRatio;
            if (ageRatio > 1.0f) continue;

            int frame = static_cast<int>(particle.age * config.animSpeed) % 2;
            // hardcoded like sprite ranks and variants until cppoorkl supports custom renderers  - alice
            int originalRank = static_cast<int>(particle.rank);
            int scaledRank = qBound(0, qRound(originalRank * invAgeRatio), 2);

            QString key = QString("sparkle_%1_%2_%3")
                    .arg(scaledRank)
                    .arg(static_cast<int>(particle.variant))
                    .arg(frame);
            auto it = spriteRects.find(key);
            if (it == spriteRects.end()) continue;
            const SpriteFrame &frameData = it.value();

            float scaledScale = config.scale * invAgeRatio * (1.0f + config.shinePower * std::sin(particle.age * config.shineSpeed)); // 666
            QRect sourceRect = frameData.rect;
            QPointF offset = frameData.offset * scaledScale;

            int scaledW = sourceRect.width() * scaledScale;
            int scaledH = sourceRect.height() * scaledScale;

            QRectF target(particle.pos.x() - offset.x(),
                          particle.pos.y() - offset.y(),
                          scaledW, scaledH);

            p.drawPixmap(target, atlas, sourceRect);
        }
    }

private slots:
    void onTimeout() {
        float dt = frameTimer.restart() / 1000.0f;
        frameCount++;
        elapsedTime += dt;
        if (elapsedTime >= 1.0f) {
            currentFPS = frameCount;
            frameCount = 0;
            elapsedTime = 0.0f;
        }
        if (fpsLabel)
            fpsLabel->setText(QString("FPS: %1").arg(currentFPS));
        if (particleCountLabel)
            particleCountLabel->setText(QString("Particles: %1").arg(particles.size()));
        QPoint globalPos = QCursor::pos();
        QPoint localPos = mapFromGlobal(globalPos);
        if (lastPos.isNull()) {
            lastPos = localPos;
            lastLastPos = localPos;
            lastLastLastPos = localPos;
            return;
        }

        if (localPos != lastPos) {
            QPointF delta = catmullRomDerivative(lastLastLastPos, lastLastPos, lastPos, localPos, 0);
            int count = ((std::min))(int(delta.manhattanLength() / config.spawnSpeed), static_cast<int>(config.spawnLimit));
            // should be a euclidean distance function  - alice

            for (int i = 0; i < count; ++i) {
                float t = i / float(count);
                QPointF pos = catmullRom(lastLastLastPos, lastLastPos, lastPos, localPos, t);

                QPointF vel = (catmullRomDerivative(lastLastLastPos, lastLastPos, lastPos, localPos, t) * config.wind) +
                              (QPointF(
                                   static_cast<float>(QRandomGenerator::global()->generateDouble() - 0.5),
                                   static_cast<float>(QRandomGenerator::global()->generateDouble() - 0.5)) * config.
                               spawnWind);

                Rank rank = static_cast<Rank>(QRandomGenerator::global()->bounded(0, 3));
                Variant variant = static_cast<Variant>(QRandomGenerator::global()->bounded(0, 2));
                particles.push_back({
                    pos, vel, static_cast<float>(QRandomGenerator::global()->generateDouble() - 0.5) * config.lifeDelta,
                    rank, variant
                });
            }

            while (particles.size() > config.maxParticles)
                particles.pop_front();
        }

        for (auto &p: particles) {
            p.velocity += QPointF(
                static_cast<float>(QRandomGenerator::global()->generateDouble() - 0.5) * config.jitter * dt,
                static_cast<float>(QRandomGenerator::global()->generateDouble() - 0.5) * config.jitter * dt);

            p.pos += p.velocity * dt;
            QRect screenRect = this->rect();

            if (config.bounce > 0.0f) {
                if (!screenRect.contains(p.pos.toPoint())) {
                    p.age = config.lifetime + 1.0f;
                }
            } else {
                if (p.pos.x() < 0.0f) {
                    p.pos.setX(0.0f);
                    p.velocity.setX(p.velocity.x() * config.bounce);
                } else if (p.pos.x() > screenRect.width()) {
                    p.pos.setX(screenRect.width());
                    p.velocity.setX(p.velocity.x() * config.bounce);
                }

                if (p.pos.y() < 0.0f) {
                    p.pos.setY(0.0f);
                    p.velocity.setY(p.velocity.y() * config.bounce);
                } else if (p.pos.y() > screenRect.height()) {
                    p.pos.setY(screenRect.height());
                    p.velocity.setY(p.velocity.y() * config.bounce);
                }
            }

            p.velocity *= config.friction; // TODO: these should be affected by dt too but im alazy  - alice
            p.velocity.setX(p.velocity.x() + config.gravityX);
            p.velocity.setY(p.velocity.y() + config.gravityY);
            p.age += dt;
        }

        while (!particles.empty() && particles.front().age > config.lifetime) {
            particles.pop_front();
        }

        lastLastLastPos = lastLastPos;
        lastLastPos = lastPos;
        lastPos = localPos;

        update();
    }

private:
    QPointF catmullRom(QPointF p0, QPointF p1, QPointF p2, QPointF p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;

        return 0.5f * (
                   (2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
               );
    }

    QPointF catmullRomDerivative(QPointF p0, QPointF p1, QPointF p2, QPointF p3, float t) {
        float t2 = t * t;

        return 0.5f * (
                   (-p0 + p2) +
                   2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
                   3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2
               );
    }

    QImage applyColourTransform(QImage img, float redFactor, float greenFactor, float blueFactor) {
        if (img.format() != QImage::Format_RGBA8888) {
            img = img.convertToFormat(QImage::Format_RGBA8888);
        }

        uchar *data = img.bits();
        int numPixels = img.width() * img.height();

        for (int i = 0; i < numPixels; ++i) {
            int idx = i * 4;
            uchar r = data[idx];
            uchar g = data[idx + 1];
            uchar b = data[idx + 2];

            data[idx] = ((std::min))(255.0f, g + (redFactor * r)); // g because r is used as a mask  - alice
            data[idx + 1] = (std::min)(255.0f, g + (greenFactor * r));
            data[idx + 2] = (std::min)(255.0f, b + (blueFactor * r));
            if (r == 255.0f && g == 255.0f && b == 255.0f) {
                data[idx + 3] = std::clamp(data[idx + 3] * config.whiteOutline, 0.0f, 255.0f);
            } else if (r == 0.0f && g == 0.0f && b == 0.0f) {
                data[idx + 3] = std::clamp(data[idx + 3] * config.blackOutline, 0.0f, 255.0f);
            }
        }
        return img;
    }

    QTimer *timer;
    std::deque<Particle> particles; // should probably be some better class than deque, maybe object pooled  - alice
    QPoint lastPos;
    QPoint lastLastPos;
    QPoint lastLastLastPos; // peak naming convention  - alice
    QPixmap atlas;
    CPPoorklMenuWidget *menu;
    QMap<QString, SpriteFrame> spriteRects;
    ParticleConfig config;
    QElapsedTimer frameTimer;
    int frameCount = 0;
    float elapsedTime = 0.0f;
    int currentFPS = 0;
    QLabel *fpsLabel = nullptr;
    QLabel *particleCountLabel = nullptr;
    QListWidget *browserList = nullptr;
    QListWidget *fileList = nullptr;
    QFileSystemWatcher* watcher;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    try {
        ParticleWidget w;
        w.showFullScreen();
        return app.exec();
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "fatal error", e.what());
    } catch (...) {
        QMessageBox::critical(nullptr, "fatal error", "unknown exception occurred.");
    }
    return app.exec();
}

#include "main.moc"
