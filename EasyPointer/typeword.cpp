#include "typeword.h"
#include "ui_typeword.h"
#include<QListView>

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QClipboard>
#include <QStandardPaths>

#include <QDateTime>

static QSettings Set(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/AIMouseSetting.ini",QSettings::IniFormat) ;

static QString strLangMother = R"(
{
"Language":[
    {"id":1,"name":"普通话","language":"zh_cn","accent":"mandarin"},
    {"id":2,"name":"英语","language":"en_us","accent":"mandarin"},
    {"id":3,"name":"广东话","language":"zh_cn_cantonese","accent":"cantonese"},
    {"id":4,"name":"上海话","language":"zh_cn_shanghainese","accent":"shanghainese"},
    {"id":5,"name":"闽南话","language":"zh_cn_minnanese","accent":"minnanese"},
    {"id":6,"name":"四川话","language":"zh_cn_lmz","accent":"lmz"},
    {"id":7,"name":"台湾话","language":"zh_cn_taiwanese","accent":"taiwanese"},
    {"id":8,"name":"罗马尼亚语","language":"ro_ro","accent":"mandarin"},
    {"id":9,"name":"尼泊尔语","language":"ne_np","accent":"mandarin"},
    {"id":10,"name":"彝语","language":"zh_cn_yinese","accent":"yinese"},
    {"id":11,"name":"印地语","language":"hi_in","accent":"mandarin"},
    {"id":12,"name":"哈萨克语","language":"kk_KZ","accent":"mandarin"},
    {"id":13,"name":"日语","language":"ja_jp","accent":"mandarin"},
    {"id":14,"name":"瑞典语","language":"sv_SE","accent":"mandarin"},
    {"id":15,"name":"波兰语","language":"pl_pl","accent":"mandarin"},
    {"id":16,"name":"斯洛文尼亚语","language":"sl_si","accent":"mandarin"},
    {"id":17,"name":"祖鲁语","language":"zu_za","accent":"mandarin"},
    {"id":18,"name":"老挝语","language":"lo_LA","accent":"mandarin"},
    {"id":19,"name":"内蒙古语","language":"mn_cn","accent":"mandarin"},
    {"id":20,"name":"挪威语","language":"nb_NO","accent":"mandarin"},
    {"id":21,"name":"法语","language":"fr_fr","accent":"mandarin"},
    {"id":22,"name":"西班牙语","language":"es_es","accent":"mandarin"},
    {"id":23,"name":"乌尔都语","language":"ur_IN","accent":"mandarin"},
    {"id":24,"name":"蒙古语（外蒙）","language":"mn_MN","accent":"mandarin"},
    {"id":25,"name":"普什图语","language":"ps_ar","accent":"mandarin"},
    {"id":26,"name":"马拉地语","language":"mr_in","accent":"mandarin"},
    {"id":27,"name":"豪萨语","language":"ha_NG","accent":"mandarin"},
    {"id":28,"name":"波斯语","language":"fa_IR","accent":"mandarin"},
    {"id":29,"name":"丹麦语","language":"da_DK","accent":"mandarin"},
    {"id":30,"name":"印尼语","language":"id_ID","accent":"mandarin"},
    {"id":31,"name":"匈牙利语","language":"hu_hu","accent":"mandarin"},
    {"id":32,"name":"爪哇语","language":"jv_ID","accent":"mandarin"},
    {"id":33,"name":"哈萨克语（内哈）","language":"kk_cn","accent":"mandarin"},
    {"id":34,"name":"泰卢固语","language":"te_in","accent":"mandarin"},
    {"id":35,"name":"希伯来语","language":"he_il","accent":"mandarin"},
    {"id":36,"name":"冰岛语","language":"is_IS","accent":"mandarin"},
    {"id":37,"name":"斯洛伐克语","language":"sk_sk","accent":"mandarin"},
    {"id":38,"name":"塞尔维亚语","language":"sr_RS","accent":"mandarin"},
    {"id":39,"name":"德语","language":"de_DE","accent":"mandarin"},
    {"id":40,"name":"格鲁吉亚语","language":"ka_ge","accent":"mandarin"},
    {"id":41,"name":"克罗地亚语","language":"hr_hr","accent":"mandarin"},
    {"id":42,"name":"乌兹别克语","language":"uz_UZ","accent":"mandarin"},
    {"id":43,"name":"韩语","language":"ko_kr","accent":"mandarin"},
    {"id":44,"name":"马来语","language":"ms_MY","accent":"mandarin"},
    {"id":45,"name":"菲律宾语","language":"fil_PH","accent":"mandarin"},
    {"id":46,"name":"保加利亚语","language":"bg_bg","accent":"mandarin"},
    {"id":47,"name":"泰米尔语","language":"ta_in","accent":"mandarin"},
    {"id":48,"name":"土耳其语","language":"tr_TR","accent":"mandarin"},
    {"id":49,"name":"斯瓦希里语","language":"sw_KE","accent":"mandarin"},
    {"id":50,"name":"荷兰语","language":"nl_NL","accent":"mandarin"},
    {"id":51,"name":"芬兰语","language":"fi_fi","accent":"mandarin"},
    {"id":52,"name":"加泰罗尼亚语","language":"ca_es","accent":"mandarin"},
    {"id":53,"name":"阿塞拜疆语","language":"az_AZ","accent":"mandarin"},
    {"id":54,"name":"南非语","language":"af_za","accent":"manddain"},
    {"id":55,"name":"捷克语","language":"cs_CZ","accent":"mandarin"},
    {"id":56,"name":"阿姆哈拉语","language":"am_et","accent":"mandarin"},
    {"id":57,"name":"意大利语","language":"it_IT","accent":"mandarin"},
    {"id":58,"name":"泰语","language":"th_TH","accent":"mandarin"},
    {"id":59,"name":"巽他语","language":"su_ID","accent":"mandarin"},
    {"id":60,"name":"越南语","language":"vi_VN","accent":"mandarin"},
    {"id":61,"name":"阿拉伯语","language":"ar_il","accent":"mandarin"},
    {"id":62,"name":"塔吉克语","language":"tg_tg","accent":"mandarin"},
    {"id":63,"name":"亚美尼亚语","language":"hy_AM","accent":"mandarin"},
    {"id":64,"name":"孟加拉语","language":"bn_BD","accent":"mandarin"},
    {"id":65,"name":"僧伽罗语","language":"si_lk","accent":"mandarin"},
    {"id":66,"name":"俄语","language":"ru-ru","accent":"mandarin"},
    {"id":67,"name":"缅甸语","language":"my_mm","accent":"mandarin"},
    {"id":68,"name":"乌克兰语","language":"uk_UA","accent":"mandarin"},
    {"id":69,"name":"马拉雅拉姆语","language":"ml_in","accent":"mandarin"},
    {"id":70,"name":"拉脱维亚语","language":"lv_lv","accent":"mandarin"},
    {"id":71,"name":"立陶宛语","language":"lt_LT","accent":"mandarin"},
    {"id":72,"name":"土库曼语","language":"tm_tk","accent":"mandarin"},
    {"id":73,"name":"高棉语","language":"km_KH","accent":"mandarin"},
    {"id":74,"name":"希腊语","language":"el_GR","accent":"mandarin"},
    {"id":75,"name":"葡萄牙语","language":"pt_PT","accent":"mandarin"}
]})" ;

static QString strTextTransMap=R"(
{
"map":[
    {"name":"普通话", "short":"cn"},
    {"name":"英语", "short":"en"},
    {"name":"日语", "short":"ja"},
    {"name":"韩语", "short":"ko"},
    {"name":"泰语", "short":"th"},
    {"name":"俄语", "short":"ru"},
    {"name":"保加利亚语", "short":"bg"},
    {"name":"乌克兰语", "short":"uk"},
    {"name":"越南语", "short":"vi"},
    {"name":"马来语", "short":"ms"},
    {"name":"印尼语", "short":"id"},
    {"name":"菲律宾语", "short":"tl"},
    {"name":"德语", "short":"de"},
    {"name":"西班牙语", "short":"es"},
    {"name":"法语", "short":"fr"},
    {"name":"捷克语", "short":"cs"},
    {"name":"豪萨语", "short":"ha"},
    {"name":"罗马尼亚语", "short":"ro"},
    {"name":"匈牙利语", "short":"hu"},
    {"name":"瑞典语", "short":"sv"},
    {"name":"斯瓦希里语", "short":"sw"},
    {"name":"荷兰语", "short":"nl"},
    {"name":"乌兹别克语", "short":"uz_UZ"},
    {"name":"波兰语", "short":"pl"},
    {"name":"祖鲁语", "short":"zu"},
    {"name":"阿拉伯语", "short":"ar"},
    {"name":"希腊语", "short":"el"},
    {"name":"波斯语", "short":"fa"},
    {"name":"希伯来语", "short":"he"},
    {"name":"普什图语", "short":"ps"},
    {"name":"亚美尼亚语", "short":"hy"},
    {"name":"乌尔都语", "short":"ur"},
    {"name":"格鲁吉亚语", "short":"ka"},
    {"name":"印地语", "short":"hi"},
    {"name":"广东话", "short":"yue"},
    {"name":"孟加拉语", "short":"bn"},
    {"name":"彝语", "short":"ii"},
    {"name":"外蒙语", "short":"nm"},
    {"name":"壮语", "short":"zua"},
    {"name":"外哈语", "short":"kk"},
    {"name":"内蒙语", "short":"mn"},
    {"name":"土耳其语", "short":"tr"},
    {"name":"内哈萨克语", "short":"kka"}
]})";

static QStringList pLangs0={
    "普通话",
    "英语",
    "日语",
    "韩语",
    "泰语",
    "俄语",
    "保加利亚语",
    "乌克兰语",
    "越南语",
    "马来语",
    "印尼语",
    "菲律宾语",
    "德语",
    "西班牙语",
    "法语",
    "捷克语",
    "豪萨语",
    "罗马尼亚语",
    "匈牙利语",
    "瑞典语",
    "斯瓦希里语",
    "荷兰语",
    "乌兹别克语",
    "波兰语",
    "祖鲁语",
    "阿拉伯语",
    "希腊语",
    "波斯语",
    "希伯来语",
    "普什图语",
    "亚美尼亚语",
    "乌尔都语",
    "格鲁吉亚语",
    "印地语",
    "广东话",
    "孟加拉语",
    "彝语",
    "外蒙语",
    "壮语",
    "外哈语",
    "内蒙语",
    "土耳其语",
    "内哈萨克语"
};

static QStringList pLangs1={
    "普通話",
    "英語",
    "日語",
    "韓語",
    "泰語",
    "俄語",
    "保加利亞文",
    "烏克蘭文",
    "越南語",
    "馬來語",
    "印尼語",
    "菲律賓文",
    "德語",
    "西班牙文",
    "法語",
    "捷克語",
    "豪薩語",
    "羅馬尼亞文",
    "匈牙利語",
    "瑞典文",
    "斯瓦希里語",
    "荷蘭語",
    "烏茲別克文",
    "波蘭語",
    "祖魯語",
    "阿拉伯文",
    "希臘語",
    "波斯文",
    "希伯來語",
    "普什圖語",
    "亞美尼亞文",
    "烏爾都語",
    "喬治亞語",
    "印地語",
    "廣東話",
    "孟加拉文",
    "彝語",
    "外蒙語",
    "壯語",
    "外哈語",
    "內蒙語",
    "土耳其文",
    "內哈薩克語"
};


static QStringList pLangs2={
    "Mandarin",
    "English",
    "Japanese",
    "Korean",
    "Thai",
    "Russian",
    "Bulgaria",
    "Ukraine",
    "Viet Nam",
    "Malay",
    "Indonesian",
    "Filipino",
    "German",
    "Spain",
    "French",
    "Czech",
    "Hausa",
    "Romania",
    "Hungary",
    "Sweden",
    "Swahili",
    "Netherlands",
    "Uzbek",
    "Poland",
    "Zulu",
    "Arabic",
    "Greece",
    "Persian",
    "Hebrew",
    "Pashto",
    "Armenia",
    "Urdu",
    "Georgia",
    "Hindi",
    "Cantonese",
    "Bengali",
    "Yi language",
    "Outer Mongolian",
    "Zhuang Language",
    "Wai Ha language",
    "Inner Mongolian",
    "Turkey",
    "Inner Kazakh"
};

static QStringList pLangs3={
    "북경어",
    "영어",
    "일본어",
    "한국어",
    "태국어",
    "러시아어",
    "불가리아",
    "우크라이나",
    "베트남",
    "말레이어",
    "인도네시아어",
    "필리핀 사람",
    "독일어",
    "스페인",
    "프랑스어",
    "체코어",
    "하우사",
    "루마니아",
    "헝가리",
    "스웨덴",
    "스와힐리어",
    "네덜란드",
    "우즈베크어",
    "폴란드",
    "줄루어",
    "아랍어",
    "그리스",
    "페르시아어",
    "히브리어",
    "파슈토어",
    "아르메니아",
    "우르두어",
    "조지아",
    "힌디어",
    "광둥어",
    "벵골어",
    "이족 언어",
    "외몽골어",
    "좡어",
    "와이 하 언어",
    "내몽골어",
    "터키",
    "내카자흐어"
};


static QStringList pLangs4={
    "Mandarin",
    "Englisch",
    "Japanisch",
    "Koreanisch",
    "Thai",
    "Russisch",
    "Bulgarien",
    "Ukraine",
    "Vietnam",
    "Malaiisch",
    "Indonesisch",
    "Philippinisch",
    "Deutsch",
    "Spanien",
    "Französisch",
    "Tschechisch",
    "Hausa",
    "Rumänien",
    "Ungarn",
    "Schweden",
    "Swahili",
    "Niederlande",
    "Usbekisch",
    "Polen",
    "Zulu",
    "Arabisch",
    "Griechenland",
    "Persisch",
    "Hebräisch",
    "Paschtu",
    "Armenien",
    "Urdu",
    "Georgien",
    "Hindi",
    "Kantonesisch",
    "Bengalisch",
    "Yi-Sprache",
    "Äußere Mongolisch",
    "Zhuang-Sprache",
    "Wai Ha Sprache",
    "Innere Mongolisch",
    "Türkei",
    "Inneres Kasachisch"
};

static QStringList pLangs5={
    "Mandarin",
    "Anglais",
    "Japonais",
    "coréen",
    "Thaï",
    "Russe",
    "Bulgarie",
    "Ukraine",
    "Viet Nam",
    "Malais",
    "indonésien",
    "Philippin",
    "Allemand",
    "Espagne",
    "Français",
    "Tchèque",
    "Haoussa",
    "Roumanie",
    "Hongrie",
    "Suède",
    "Swahili",
    "Pays-Bas",
    "Ouzbek",
    "Pologne",
    "Zoulou",
    "Arabe",
    "Grèce",
    "Persan",
    "Hébreu",
    "Pachtoune",
    "Arménie",
    "ourdou",
    "Géorgie",
    "Hindi",
    "cantonais",
    "Bengali",
    "Langue Yi",
    "Mongol extérieur",
    "Langue Zhuang",
    "Langue Wai Ha",
    "Mongol intérieur",
    "Turquie",
    "Kazakh intérieur"
};

static QStringList pLangs6 = {
    "マンダリン",
    "英語",
    "日本人",
    "韓国語",
    "タイ語",
    "ロシア語",
    "ブルガリア",
    "ウクライナ",
    "ベトナム",
    "マレー語",
    "インドネシア語",
    "フィリピン人",
    "ドイツ語",
    "スペイン",
    "フランス語",
    "チェコ語",
    "ハウサ語",
    "ルーマニア",
    "ハンガリー",
    "スウェーデン",
    "スワヒリ語",
    "オランダ",
    "ウズベキスタン",
    "ポーランド",
    "ズールー語",
    "アラビア語",
    "ギリシャ",
    "ペルシャ語",
    "ヘブライ語",
    "パシュトー語",
    "アルメニア",
    "ウルドゥー語",
    "ジョージア",
    "ヒンディー語",
    "広東語",
    "ベンガル語",
    "イ語",
    "外モンゴル語",
    "チワン語",
    "ワイハ語",
    "内モンゴル語",
    "トルコ",
    "内カザフスタン"
};

static QStringList pLangs7={
    "Mandarín",
    "Inglés",
    "Japonés",
    "Coreano",
    "Tailandés",
    "Ruso",
    "Bulgaria",
    "Ucrania",
    "Viet Nam",
    "Malayo",
    "Indonesio",
    "Filipino",
    "Alemán",
    "España",
    "Francés",
    "Checo",
    "Hausa",
    "Rumanía",
    "Hungría",
    "Suecia",
    "Swahili",
    "Países Bajos",
    "Uzbeko",
    "Polonia",
    "Zulú",
    "Árabe",
    "Grecia",
    "Persa",
    "Hebreo",
    "Pastún",
    "Armenia",
    "Urdu",
    "Georgia",
    "Hindi",
    "Cantonés",
    "Bengalí",
    "Idioma Yi",
    "Mongol exterior",
    "Idioma Zhuang",
    "Idioma Wai Ha",
    "Mongol Interior",
    "Turquía",
    "Kazajo interior"
} ;

static QStringList pLangs8={
    "Mandarim",
    "Inglês",
    "Japonês",
    "Coreano",
    "Tailandês",
    "Russo",
    "Bulgária",
    "Ucrânia",
    "Vietnã",
    "Malaio",
    "Indonésio",
    "Filipino",
    "Alemão",
    "Espanha",
    "Francês",
    "Tcheco",
    "Hausa",
    "Roménia",
    "Hungria",
    "Suécia",
    "Suaíli",
    "Países Baixos",
    "Uzbeque",
    "Polónia",
    "Zulu",
    "Árabe",
    "Grécia",
    "Persa",
    "hebraico",
    "Pashto",
    "Arménia",
    "Urdu",
    "Geórgia",
    "Hindi",
    "cantonês",
    "Bengali",
    "Língua Yi",
    "Mongólia Exterior",
    "Língua Zhuang",
    "Língua Wai Ha",
    "Mongol Interior",
    "Turquia",
    "Cazaque Interior"
} ;

static QStringList pLangs9={
    "Мандарин",
    "Английский",
    "японский",
    "Корейский",
    "Тайский",
    "русский",
    "Болгария",
    "Украина",
    "Вьетнам",
    "Малайский",
    "индонезийский",
    "Филиппинец",
    "немецкий",
    "Испания",
    "Французский",
    "чешский",
    "Хауса",
    "Румыния",
    "Венгрия",
    "Швеция",
    "Суахили",
    "Нидерланды",
    "узбекский",
    "Польша",
    "Зулус",
    "арабский",
    "Греция",
    "персидский",
    "иврит",
    "пушту",
    "Армения",
    "урду",
    "Грузия",
    "хинди",
    "кантонский",
    "бенгальский",
    "Язык И",
    "Внешняя Монголия",
    "Чжуанский язык",
    "Язык Вай Ха",
    "Внутренний монгольский",
    "Индейка",
    "Внутренний казахский"
} ;

static QStringList pLangs10={
    "Mandarin",
    "İngilizce",
    "Japonca",
    "Koreli",
    "Tayca",
    "Rusça",
    "Bulgaristan",
    "Ukrayna",
    "Vietnam",
    "Malayca",
    "Endonezyaca",
    "Filipinli",
    "Almanca",
    "İspanya",
    "Fransız",
    "Çek",
    "Hausa",
    "Romanya",
    "Macaristan",
    "İsveç",
    "Svahili",
    "Hollanda",
    "Özbek",
    "Polonya",
    "Zulu",
    "Arapça",
    "Yunanistan",
    "Farsça",
    "İbranice",
    "Peştuca",
    "Ermenistan",
    "Urduca",
    "Gürcistan",
    "Hintçe",
    "Kantonca",
    "Bengalce",
    "Yi dili",
    "Dış Moğolca",
    "Zhuang Dili",
    "Wai Ha dili",
    "İç Moğolca",
    "Türkiye",
    "İç Kazakça"
} ;


static QString strMLang0= R"(
{
"namegroup":[
"普通话",
"英语",
"广东话",
"上海话",
"闽南话",
"四川话",
"台湾话",
"罗马尼亚语",
"尼泊尔语",
"彝语",
"印地语",
"哈萨克语",
"日语",
"瑞典语",
"波兰语",
"斯洛文尼亚语",
"祖鲁语",
"老挝语",
"内蒙古语",
"挪威语",
"法语",
"西班牙语",
"乌尔都语",
"蒙古语（外蒙）",
"普什图语",
"马拉地语",
"豪萨语",
"波斯语",
"丹麦语",
"印尼语",
"匈牙利语",
"爪哇语",
"哈萨克语（内哈）",
"泰卢固语",
"希伯来语",
"冰岛语",
"斯洛伐克语",
"塞尔维亚语",
"德语",
"格鲁吉亚语",
"克罗地亚语",
"乌兹别克语",
"韩语",
"马来语",
"菲律宾语",
"保加利亚语",
"泰米尔语",
"土耳其语",
"斯瓦希里语",
"荷兰语",
"芬兰语",
"加泰罗尼亚语",
"阿塞拜疆语",
"南非语",
"捷克语",
"阿姆哈拉语",
"意大利语",
"泰语",
"巽他语",
"越南语",
"阿拉伯语",
"塔吉克语",
"亚美尼亚语",
"孟加拉语",
"僧伽罗语",
"俄语",
"缅甸语",
"乌克兰语",
"马拉雅拉姆语",
"拉脱维亚语",
"立陶宛语",
"土库曼语",
"高棉语",
"希腊语",
"葡萄牙语"
]})";

static QString strMLang1= R"(
{
"namegroup":[
"普通話",
"英語",
"廣東話",
"上海話",
"閩南話",
"四川話",
"臺灣話",
"羅馬尼亞文",
"尼泊爾語",
"彝語",
"印地語",
"哈薩克文",
"日語",
"瑞典文",
"波蘭語",
"斯洛維尼亞語",
"祖魯語",
"老撾語",
"內蒙古文",
"挪威文",
"法語",
"西班牙文",
"烏爾都語",
"蒙古語（外蒙）",
"普什圖語",
"馬拉地語",
"豪薩語",
"波斯文",
"丹麥文",
"印尼語",
"匈牙利語",
"爪哇語",
"哈薩克語（內哈）",
"泰盧固語",
"希伯來語",
"冰島語",
"斯洛伐克語",
"塞爾維亞文",
"德語",
"喬治亞語",
"克羅埃西亞語",
"烏茲別克文",
"韓語",
"馬來語",
"菲律賓文",
"保加利亞文",
"泰米爾文",
"土耳其文",
"斯瓦希里語",
"荷蘭語",
"芬蘭文",
"加泰隆尼亞文",
"亞塞拜然語",
"南非語",
"捷克語",
"阿姆哈拉語",
"義大利語",
"泰語",
"巽他語",
"越南語",
"阿拉伯文",
"塔吉克文",
"亞美尼亞文",
"孟加拉文",
"僧伽羅語",
"俄語",
"緬甸文",
"烏克蘭文",
"馬拉雅拉姆文",
"拉脫維亞語",
"立陶宛語",
"土庫曼文",
"高棉語",
"希臘語",
"葡萄牙文"
]})";

static QString strMLang2= R"(
{
"namegroup":[
"Mandarin",
"English",
"Cantonese",
"Shanghainese",
"Hokkien dialect",
"Sichuan dialect",
"Taiwan",
"Romania",
"Nepal",
"Yi language",
"Hindi",
"Kazakh",
"Japanese",
"Sweden",
"Poland",
"Slovenia",
"Zulu",
"Lao",
"Inner Mongolia",
"Norway",
"French",
"Spain",
"Urdu",
"Mongolia (Outer Mongolia）",
"Pashto",
"Marathi",
"Hausa",
"Persian",
"Denmark",
"Indonesian",
"Hungary",
"Javanese",
"Kazakh (Neha）",
"Telugu",
"Hebrew",
"Iceland",
"Slovakia",
"Serbia",
"German",
"Georgia",
"Croatia",
"Uzbek",
"Korean",
"Malay",
"Filipino",
"Bulgaria",
"Tamil",
"Turkey",
"Swahili",
"Netherlands",
"Finland",
"Catalan",
"Azerbaijan",
"South Africa",
"Czech",
"Amharic",
"Italy",
"Thai",
"Sundanese",
"Viet Nam",
"Arabic",
"Tajik",
"Armenia",
"Bengali",
"Sinhala",
"Russian",
"Burmese",
"Ukraine",
"Malayalam",
"Latvian",
"Lithuania",
"Turkmen",
"Khmer",
"Greece",
"Portugal"
]})";

static QString strMLang3= R"(
{
"namegroup":[
"북경어",
"영어",
"광둥어",
"상하이 사람",
"호키엔 방언",
"쓰촨 방언",
"대만",
"루마니아",
"네팔",
"이족 언어",
"힌디어",
"카자흐어",
"일본어",
"스웨덴",
"폴란드",
"슬로베니아",
"줄루어",
"라오스어",
"내몽골",
"노르웨이",
"프랑스어",
"스페인",
"우르두어",
"몽골(외몽골）",
"파슈토어",
"마라티어",
"하우사",
"페르시아어",
"덴마크",
"인도네시아어",
"헝가리",
"자바어",
"카자흐어(네하）",
"텔루구어",
"히브리어",
"아이슬란드",
"슬로바키아",
"세르비아",
"독일어",
"조지아",
"크로아티아",
"우즈베크어",
"한국어",
"말레이어",
"필리핀 사람",
"불가리아",
"타밀어",
"터키",
"스와힐리어",
"네덜란드",
"핀란드",
"카탈로니아어",
"아제르바이잔",
"남아프리카 공화국",
"체코어",
"암하라어",
"이탈리아",
"태국어",
"순다족",
"베트남",
"아랍어",
"타지크어",
"아르메니아",
"벵골어",
"싱할라어",
"러시아어",
"버마어",
"우크라이나",
"말라얄람어",
"라트비아어",
"리투아니아",
"투르크멘어",
"크메르어",
"그리스",
"포르투갈"
]})";

static QString strMLang4= R"(
{
"namegroup":[
"Mandarin",
"Englisch",
"Kantonesisch",
"Shanghaier",
"Hokkien-Dialekt",
"Sichuan-Dialekt",
"Taiwan",
"Rumänien",
"Nepal",
"Yi-Sprache",
"Hindi",
"Kasachisch",
"Japanisch",
"Schweden",
"Polen",
"Slowenien",
"Zulu",
"Laotisch",
"Innere Mongolei",
"Norwegen",
"Französisch",
"Spanien",
"Urdu",
"Mongolei (Äußere Mongolei）",
"Paschtu",
"Marathi",
"Hausa",
"Persisch",
"Dänemark",
"Indonesisch",
"Ungarn",
"Javanisch",
"Kasachisch (Neha）",
"Telugu",
"Hebräisch",
"Island",
"Slowakei",
"Serbien",
"Deutsch",
"Georgien",
"Kroatien",
"Usbekisch",
"Koreanisch",
"Malaiisch",
"Philippinisch",
"Bulgarien",
"Tamil",
"Türkei",
"Swahili",
"Niederlande",
"Finnland",
"Katalanisch",
"Aserbaidschan",
"Südafrika",
"Tschechisch",
"Amharisch",
"Italien",
"Thai",
"Sundanesisch",
"Vietnam",
"Arabisch",
"Tadschikisch",
"Armenien",
"Bengalisch",
"Singhalesisch",
"Russisch",
"Birmanisch",
"Ukraine",
"Malayalam",
"Lettisch",
"Litauen",
"Turkmenisch",
"Khmer",
"Griechenland",
"Portugal"
]})";

static QString strMLang5= R"(
{
"namegroup":[
"Mandarin",
"Anglais",
"cantonais",
"Shanghaïen",
"Dialecte hokkien",
"Dialecte du Sichuan",
"Taïwan",
"Roumanie",
"Népal",
"Langue Yi",
"Hindi",
"Kazakh",
"Japonais",
"Suède",
"Pologne",
"Slovénie",
"Zoulou",
"Laos",
"Mongolie intérieure",
"Norvège",
"Français",
"Espagne",
"ourdou",
"Mongolie (Mongolie extérieure）",
"Pachtoune",
"Marathi",
"Haoussa",
"Persan",
"Danemark",
"indonésien",
"Hongrie",
"Javanais",
"Kazakh (Neha）",
"Telugu",
"Hébreu",
"Islande",
"Slovaquie",
"Serbie",
"Allemand",
"Géorgie",
"Croatie",
"Ouzbek",
"coréen",
"Malais",
"Philippin",
"Bulgarie",
"Tamoul",
"Turquie",
"Swahili",
"Pays-Bas",
"Finlande",
"Catalan",
"Azerbaïdjan",
"Afrique du Sud",
"Tchèque",
"Amharique",
"Italie",
"Thaï",
"Sundanais",
"Viet Nam",
"Arabe",
"Tadjik",
"Arménie",
"Bengali",
"Cinghalais",
"Russe",
"Birman",
"Ukraine",
"Malayalam",
"Letton",
"Lituanie",
"Turkmène",
"Khmer",
"Grèce",
"Portugal"
]})";

static QString strMLang6= R"(
{
"namegroup":[
"マンダリン",
"英語",
"広東語",
"上海語",
"福建方言",
"四川方言",
"台湾",
"ルーマニア",
"ネパール",
"イ語",
"ヒンディー語",
"カザフスタン",
"日本人",
"スウェーデン",
"ポーランド",
"スロベニア",
"ズールー語",
"ラオス",
"内モンゴル自治区",
"ノルウェー",
"フランス語",
"スペイン",
"ウルドゥー語",
"モンゴル(外モンゴル自治区）",
"パシュトー語",
"マラーティー語",
"ハウサ語",
"ペルシャ語",
"デンマーク",
"インドネシア語",
"ハンガリー",
"ジャワ語",
"カザフスタン(ネハ）",
"テルグ語",
"ヘブライ語",
"アイスランド",
"スロバキア",
"セルビア",
"ドイツ語",
"ジョージア",
"クロアチア",
"ウズベキスタン",
"韓国語",
"マレー語",
"フィリピン人",
"ブルガリア",
"タミル語",
"トルコ",
"スワヒリ語",
"オランダ",
"フィンランド",
"カタルーニャ語",
"アゼルバイジャン",
"南アフリカ",
"チェコ語",
"アムハラ語",
"イタリア",
"タイ語",
"スンダ人",
"ベトナム",
"アラビア語",
"タジク語",
"アルメニア",
"ベンガル語",
"シンハラ語",
"ロシア語",
"ビルマ語",
"ウクライナ",
"マラヤーラム語",
"ラトビア語",
"リトアニア",
"トルクメン",
"クメール語",
"ギリシャ",
"ポルトガル"
]})";

static QString strMLang7= R"(
{
"namegroup":[
"Mandarín",
"Inglés",
"Cantonés",
"Shanghainés",
"Dialecto Hokkien",
"Dialecto de Sichuan",
"Taiwán",
"Rumanía",
"Nepal",
"Idioma Yi",
"Hindi",
"Kazajo",
"Japonés",
"Suecia",
"Polonia",
"Eslovenia",
"Zulú",
"Lao",
"Mongolia Interior",
"Noruega",
"Francés",
"España",
"Urdu",
"Mongolia (Mongolia Exterior）",
"Pastún",
"Marathi",
"Hausa",
"Persa",
"Dinamarca",
"Indonesio",
"Hungría",
"Javanés",
"Kazajo (Neha）",
"Telugu",
"Hebreo",
"Islandia",
"Eslovaquia",
"Serbia",
"Alemán",
"Georgia",
"Croacia",
"Uzbeko",
"Coreano",
"Malayo",
"Filipino",
"Bulgaria",
"Tamil",
"Turquía",
"Swahili",
"Países Bajos",
"Finlandia",
"Catalán",
"Azerbaiyán",
"Sudáfrica",
"Checo",
"Amárico",
"Italia",
"Tailandés",
"Sundanese",
"Viet Nam",
"Árabe",
"Tayiko",
"Armenia",
"Bengalí",
"Cingales",
"Ruso",
"Birmano",
"Ucrania",
"Malayalam",
"Letón",
"Lituania",
"Turcomano",
"Khmer",
"Grecia",
"Portugal"
]})";

static QString strMLang8= R"(
{
"namegroup":[
"Mandarim",
"Inglês",
"cantonês",
"Xangai",
"Dialeto Hokkien",
"Dialeto de Sichuan",
"Taiwan",
"Roménia",
"Nepal",
"Língua Yi",
"Hindi",
"Cazaque",
"Japonês",
"Suécia",
"Polónia",
"Eslovénia",
"Zulu",
"Laos",
"Mongólia Interior",
"Noruega",
"Francês",
"Espanha",
"Urdu",
"Mongólia (Mongólia Exterior）",
"Pashto",
"Marathi",
"Hausa",
"Persa",
"Dinamarca",
"Indonésio",
"Hungria",
"javanês",
"Cazaque (Neha）",
"Telugu",
"hebraico",
"Islândia",
"Eslováquia",
"Sérvia",
"Alemão",
"Geórgia",
"Croácia",
"Uzbeque",
"Coreano",
"Malaio",
"Filipino",
"Bulgária",
"Tâmil",
"Turquia",
"Suaíli",
"Países Baixos",
"Finlândia",
"catalão",
"Azerbaijão",
"África do Sul",
"Tcheco",
"Amárico",
"Itália",
"Tailandês",
"Sundanês",
"Vietnã",
"Árabe",
"Tadjique",
"Arménia",
"Bengali",
"cingalês",
"Russo",
"birmanês",
"Ucrânia",
"Malaiala",
"letão",
"Lituânia",
"Turcomano",
"Khmer",
"Grécia",
"Portugal"
]})";

static QString strMLang9= R"(
{
"namegroup":[
"Мандарин",
"Английский",
"кантонский",
"Шанхайский",
"Диалект Хоккиен",
"сычуаньский диалект",
"Тайвань",
"Румыния",
"Непал",
"Язык И",
"хинди",
"казахский",
"японский",
"Швеция",
"Польша",
"Словения",
"Зулус",
"Лаос",
"Внутренняя Монголия",
"Норвегия",
"Французский",
"Испания",
"урду",
"Монголия (Внешняя Монголия）",
"пушту",
"Маратхи",
"Хауса",
"персидский",
"Дания",
"индонезийский",
"Венгрия",
"яванский",
"Казах (Неха）",
"Телугу",
"иврит",
"Исландия",
"Словакия",
"Сербия",
"немецкий",
"Грузия",
"Хорватия",
"узбекский",
"Корейский",
"Малайский",
"Филиппинец",
"Болгария",
"тамильский",
"Индейка",
"Суахили",
"Нидерланды",
"Финляндия",
"Каталонский",
"Азербайджан",
"Южная Африка",
"чешский",
"амхарский",
"Италия",
"Тайский",
"Сунданская",
"Вьетнам",
"арабский",
"таджикский",
"Армения",
"бенгальский",
"сингальский",
"русский",
"бирманский",
"Украина",
"Малаялам",
"латышский",
"Литва",
"Туркмен",
"кхмерский",
"Греция",
"Португалия"
]})";

static QString strMLang10= R"(
{
"namegroup":[
"Mandarin",
"İngilizce",
"Kantonca",
"Şangaylı",
"Hokkien lehçesi",
"Sichuan lehçesi",
"Tayvan",
"Romanya",
"Nepal",
"Yi dili",
"Hintçe",
"Kazak",
"Japonca",
"İsveç",
"Polonya",
"Slovenya",
"Zulu",
"Lao",
"İç Moğolistan",
"Norveç",
"Fransız",
"İspanya",
"Urduca",
"Moğolistan (Dış Moğolistan）",
"Peştuca",
"Marathi",
"Hausa",
"Farsça",
"Danimarka",
"Endonezyaca",
"Macaristan",
"Cava",
"Kazakça (Neha）",
"Telugu dili",
"İbranice",
"İzlanda",
"Slovakya",
"Sırbistan",
"Almanca",
"Gürcistan",
"Hırvatistan",
"Özbek",
"Koreli",
"Malayca",
"Filipinli",
"Bulgaristan",
"Tamilce",
"Türkiye",
"Svahili",
"Hollanda",
"Finlandiya",
"Katalanca",
"Azerbaycan",
"Güney Afrika",
"Çek",
"Amharca",
"İtalya",
"Tayca",
"Sundalı",
"Vietnam",
"Arapça",
"Tacik",
"Ermenistan",
"Bengalce",
"Sinhala",
"Rusça",
"Birmanya",
"Ukrayna",
"Malayalam",
"Letonca",
"Litvanya",
"Türkmen",
"Khmer",
"Yunanistan",
"Portekiz"
]})";

typeword::typeword(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::typeword)
{
    ui->setupUi(this);

    m_langs = QJsonDocument::fromJson(strLangMother.toUtf8().data()).object()["Language"].toArray();

    QString strTmp ;
    QStringList langs;
    int nCount = m_langs.count() ;
    for(int i=0; i<nCount; i++)
    {
        QJsonObject Item = m_langs[i].toObject() ;
        langs.append(Item["name"].toString()) ;
        //strTmp += QString("\"")+Item["name"].toString() + QString("\",\n") ;
    }
    //qDebug().noquote()<< strTmp;

    //strLangMother.remove('\n');
    //strLangMother.remove(' ');
    //strLangMother.replace("},{","},\n{");
    //qDebug().noquote()<<strLangMother;

    ui->comboBoxLang1->addItems(langs);

    langs.clear();
    m_trans = QJsonDocument::fromJson(strTextTransMap.toUtf8().data()).object()["map"].toArray() ;

    nCount = m_trans.count() ;
    for(int i=0; i<nCount; i++)
    {
        QJsonObject Item = m_trans[i].toObject() ;
        langs.append(Item["name"].toString()) ;

    }
    ui->comboBoxLang2->addItems(langs);
    ui->comboBoxLang3->addItems(langs);

    ui->comboBoxLang1->setCurrentIndex(Set.value("Lang1").toInt());
    ui->comboBoxLang2->setCurrentIndex(Set.value("Lang2").toInt());
    ui->comboBoxLang3->setCurrentIndex(Set.value("Lang3",1).toInt());

    ui->comboBoxLang1->setView(new QListView());
    ui->comboBoxLang2->setView(new QListView());
    ui->comboBoxLang3->setView(new QListView());

    connect(ui->comboBoxLang1,&QComboBox::activated,this,[](int index){Set.setValue("Lang1",index) ;}) ;
    connect(ui->comboBoxLang2,&QComboBox::activated,this,[](int index){Set.setValue("Lang2",index) ;}) ;
    connect(ui->comboBoxLang3,&QComboBox::activated,this,[](int index){Set.setValue("Lang3",index) ;}) ;

    ui->typingLog->setMaximumBlockCount(10) ;
}

typeword::~typeword()
{
    delete ui;
}

void typeword::changeEvent(QEvent *pEvt)
{
    if(pEvt->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        int nIndex = Set.value("setLang",0).toInt()  ;

        QString strMLang = strMLang0 ;
        QStringList pTLang = pLangs0 ;

        switch (nIndex) {
        case 1:
            strMLang = strMLang1 ;
            pTLang = pLangs1 ;
            break;
        case 2:
            strMLang = strMLang2 ;
            pTLang = pLangs2 ;
            break;
        case 3:
            strMLang = strMLang3 ;
            pTLang = pLangs3 ;
            break;
        case 4:
            strMLang = strMLang4 ;
            pTLang = pLangs4 ;
            break;
        case 5:
            strMLang = strMLang5 ;
            pTLang = pLangs5 ;
            break;
        case 6:
            strMLang = strMLang6 ;
            pTLang = pLangs6 ;
            break;
        case 7:
            strMLang = strMLang7 ;
            pTLang = pLangs7 ;
            break;
        case 8:
            strMLang = strMLang8 ;
            pTLang = pLangs8 ;
            break;
        case 9:
            strMLang = strMLang9 ;
            pTLang = pLangs9 ;
            break;
        case 10:
            strMLang = strMLang10 ;
            pTLang = pLangs10 ;
            break;
        default:
            break;
        }

        int nCount = ui->comboBoxLang2->count() ;
        for(int i=0; i<nCount; i++)
        {
            ui->comboBoxLang2->setItemText(i,pTLang[i]);
            ui->comboBoxLang3->setItemText(i,pTLang[i]);
        }

        QJsonArray jMLs = QJsonDocument::fromJson(strMLang.toUtf8().data()).object()["namegroup"].toArray();
        nCount = ui->comboBoxLang1->count() ;
        for(int i=0; i<nCount; i++)
        {
            ui->comboBoxLang1->setItemText(i,jMLs[i].toString());
        }

    }

    QWidget::changeEvent(pEvt) ;
}

QString typeword::getMontherLang(bool bIsTyping)
{
    if(bIsTyping)
    {
        int nIndex = ui->comboBoxLang1->currentIndex();
        QJsonObject Item = m_langs[nIndex].toObject() ;
        return Item["language"].toString();
    }

    int nIndex = ui->comboBoxLang2->currentIndex();
    QJsonObject Item = m_trans[nIndex].toObject() ;
    QString strLang = Item["name"].toString() ;

    int nCount = m_langs.count() ;
    for(int i=0; i<nCount; i++)
    {
        QJsonObject Item = m_langs[i].toObject() ;
        if(Item["name"].toString() == strLang)
            return Item["language"].toString();
    }

    return "" ;
}

QString typeword::getMontherAccent(bool bIsTyping)
{
    if(bIsTyping)
    {
        int nIndex = ui->comboBoxLang1->currentIndex();
        QJsonObject Item = m_langs[nIndex].toObject() ;
        return Item["accent"].toString();
    }

    int nIndex = ui->comboBoxLang2->currentIndex();
    QJsonObject Item = m_trans[nIndex].toObject() ;
    QString strLang = Item["name"].toString() ;

    int nCount = m_langs.count() ;
    for(int i=0; i<nCount; i++)
    {
        QJsonObject Item = m_langs[i].toObject() ;
        if(Item["name"].toString() == strLang)
            return Item["accent"].toString();
    }

    return "" ;
}

QString typeword::getTransLang1()
{
    int nIndex = ui->comboBoxLang2->currentIndex();
    QJsonObject Item = m_trans[nIndex].toObject() ;
    return Item["short"].toString();
}

QString typeword::getTransLang2()
{
    int nIndex = ui->comboBoxLang3->currentIndex();
    QJsonObject Item = m_trans[nIndex].toObject() ;
    return Item["short"].toString();
}

QString typeword::getLastText()
{
    return ui->plainTextEdit->toPlainText().trimmed() ;
}

void typeword::ShowVoiceText(const QString&strText,bool bAppend)
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(strText);

    //QClipboard *pClip = QApplication::clipboard() ;
    //pClip->setText(strText);

    //QWidget *focusWidget = QApplication::focusWidget();
    //if(focusWidget == ui->plainTextEdit)
    //   pClip->setText("");
}

void typeword::showLog(const QString&strLog,bool bEnd)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString strLine=QString("[%1] %2: %3").arg(currentDateTime.toString("MM-dd hh:mm:ss.zzz")).arg(bEnd).arg(strLog);
    ui->typingLog->appendPlainText(strLine);
}

bool typeword::autoPressReturn()
{
    return ui->checkBoxEnter->isChecked() ;
}
