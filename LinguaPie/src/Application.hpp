#pragma once
#include <memory>
#include <window/OverlayWindow.hpp>

#include "rendering/buffers/ConstantBuffer.hpp"
#include "rendering/buffers/data/FrameData.hpp"

struct Application final : EventListener<WindowEvent> {
  template <typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  Application();
  ~Application();

  bool Initialize();
  void Cleanup();

  void RunMainLoop();

  void HandleEvent(const WindowEvent& e) override;
  LRESULT HandleKeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

private:
  void Update();
  void Render() const;

private:
  bool m_isRunning;
  std::shared_ptr<OverlayWindow> m_window;
  ConstantBuffer<FrameData> m_frameBuffer;
  ShaderPipeline m_menuPipeline;

  // Only when a window is shown
  std::vector<HKL> m_layouts;
  float m_radius = 10.0f;
  float m_innerRadius = 1.0f;
  int m_segments = 4;
  int m_selectedSegment = 0;
  int m_currentSegment = 0;

  // Hooks
  bool m_isWindowShown = false;
  bool m_windowsPressed = false;
  bool m_spacePressed = false;

  inline static std::unordered_map<uint16_t, std::wstring> m_langMap = {
    { 0x0409, L"EN" },
    { 0x0419, L"RU" },
    { 0x0422, L"UA" },
    { 0x040C, L"FR" },
    { 0x0407, L"DE" },
    { 0x0410, L"IT" },
  };

  inline static std::unordered_map<uint16_t, std::wstring> m_layoutMap =  {
    {0x0401, L"Arabic (101)"},
    {0x0402, L"Bulgarian (Typewriter)"},
    {0x0404, L"Chinese (Traditional) - US Keyboard"},
    {0x0405, L"Czech"},
    {0x0406, L"Danish"},
    {0x0407, L"GE"},
    {0x0408, L"Greek"},
    {0x0409, L"US"},
    {0x040A, L"SP"},
    {0x040B, L"Finnish"},
    {0x040C, L"FR"},
    {0x040D, L"Hebrew"},
    {0x040E, L"Hungarian"},
    {0x040F, L"Icelandic"},
    {0x0410, L"Italian"},
    {0x0411, L"Japanese"},
    {0x0412, L"Korean"},
    {0x0413, L"Dutch"},
    {0x0414, L"Norwegian"},
    {0x0415, L"Polish (Programmers)"},
    {0x0416, L"Portuguese (Brazilian ABNT)"},
    {0x0418, L"Romanian (Legacy)"},
    {0x0419, L"RU"},
    {0x041A, L"Standard"},
    {0x041B, L"Slovak"},
    {0x041C, L"Albanian"},
    {0x041D, L"Swedish"},
    {0x041E, L"Thai Kedmanee"},
    {0x041F, L"Turkish Q"},
    {0x0420, L"Urdu"},
    {0x0422, L"UA"},
    {0x0423, L"Belarusian"},
    {0x0424, L"Slovenian"},
    {0x0425, L"Estonian"},
    {0x0426, L"Latvian"},
    {0x0427, L"Lithuanian IBM"},
    {0x0428, L"Tajik"},
    {0x0429, L"Persian"},
    {0x042A, L"Vietnamese"},
    {0x042B, L"Armenian Eastern"},
    {0x042C, L"Azeri Latin"},
    {0x042E, L"Sorbian Standard (Legacy)"},
    {0x042F, L"Macedonian (FYROM)"},
    {0x0432, L"Setswana"},
    {0x0437, L"Georgian"},
    {0x0438, L"Faeroese"},
    {0x0439, L"Devanagari - INSCRIPT"},
    {0x043A, L"Maltese 47-Key"},
    {0x043B, L"Norwegian with Sami"},
    {0x043F, L"Kazakh"},
    {0x0440, L"Kyrgyz Cyrillic"},
    {0x0442, L"Turkmen"},
    {0x0444, L"Tatar (Legacy)"},
    {0x0445, L"Bengali"},
    {0x0446, L"Punjabi"},
    {0x0447, L"Gujarati"},
    {0x0448, L"Oriya"},
    {0x0449, L"Tamil"},
    {0x044A, L"Telugu"},
    {0x044B, L"Kannada"},
    {0x044C, L"Malayalam"},
    {0x044D, L"Assamese - INSCRIPT"},
    {0x044E, L"Marathi"},
    {0x0450, L"Mongolian Cyrillic"},
    {0x0451, L"Tibetan (PRC)"},
    {0x0452, L"United Kingdom Extended"},
    {0x0453, L"Khmer"},
    {0x0454, L"Lao"},
    {0x045A, L"Syriac"},
    {0x045B, L"Sinhala"},
    {0x045C, L"Cherokee Nation"},
    {0x0461, L"Nepali"},
    {0x0463, L"Pashto (Afghanistan)"},
    {0x0465, L"Divehi Phonetic"},
    {0x0468, L"Hausa"},
    {0x046A, L"Yoruba"},
    {0x046C, L"Sesotho sa Leboa"},
    {0x046D, L"Bashkir"},
    {0x046E, L"Luxembourgish"},
    {0x046F, L"Greenlandic"},
    {0x0470, L"Igbo"},
    {0x0474, L"Guarani"},
    {0x0475, L"Hawaiian"},
    {0x0480, L"Uyghur (Legacy)"},
    {0x0481, L"Maori"},
    {0x0485, L"Sakha"},
    {0x0488, L"Wolof"},
    {0x0492, L"Central Kurdish"},
    {0x0804, L"Chinese (Simplified) - US Keyboard"},
    {0x0807, L"Swiss German"},
    {0x0809, L"United Kingdom"},
    {0x080A, L"Latin American"},
    {0x080C, L"Belgian French"},
    {0x0813, L"Belgian (Period)"},
    {0x0816, L"Portuguese"},
    {0x081A, L"Serbian (Latin)"},
    {0x082C, L"Azeri Cyrillic"},
    {0x083B, L"Swedish with Sami"},
    {0x0843, L"Uzbek Cyrillic"},
    {0x0850, L"Mongolian (Mongolian Script)"},
    {0x085D, L"Inuktitut - Latin"},
    {0x085F, L"Central Atlas Tamazight"},
    {0x0C04, L"Chinese (Traditional, Hong Kong S.A.R.) - US Keyboard"},
    {0x0C0C, L"Canadian French (Legacy)"},
    {0x0C1A, L"Serbian (Cyrillic)"},
    {0x0C51, L"Dzongkha"},
    {0x1004, L"Chinese (Simplified, Singapore) - US Keyboard"},
    {0x1009, L"Canadian French"},
    {0x100C, L"Swiss French"},
    {0x105F, L"Tifinagh (Basic)"},
    {0x1404, L"Chinese (Traditional, Macao S.A.R.) - US Keyboard"},
    {0x1809, L"Irish"},
    {0x201A, L"Bosnian (Cyrillic)"},
    {0x4009, L"India"},
// Added F to "Layout Id"
    {0xF028, L"Arabic (102)"},
    {0xF004, L"Bulgarian (Latin)"},
    {0xF005, L"Czech (QWERTY)"},
    {0xF012, L"German (IBM)"},
    {0xF016, L"Greek (220)"},
    {0xF002, L"United States-Dvorak"},
    {0xF086, L"Spanish Variation"},
    {0xF006, L"Hungarian 101-key"},
    {0xF003, L"Italian (142)"},
    {0xF007, L"Polish (214)"},
    {0xF010, L"Portuguese (Brazilian ABNT2)"},
    {0xF0A5, L"Romanian (Standard)"},
    {0xF008, L"Russian (Typewriter)"},
    {0xF013, L"Slovak (QWERTY)"},
    {0xF021, L"Thai Pattachote"},
    {0xF014, L"Turkish F"},
    {0xF015, L"Latvian (QWERTY)"},
    {0xF027, L"Lithuanian"},
    {0xF025, L"Armenian Western"},
    {0xF0BD, L"Azerbaijani (Standard)"},
    {0xF09F, L"Sorbian Extended"},
    {0xF0A4, L"Macedonian (FYROM) - Standard"},
    {0xF0AB, L"Georgian (QWERTY)"},
    {0xF00C, L"Hindi Traditional"},
    {0xF02B, L"Maltese 48-Key"},
    {0xF02C, L"Sami Extended Norway"},
    {0xF0BC, L"Tatar"},
    {0xF02A, L"Bengali - INSCRIPT (Legacy)"},
    {0xF0B5, L"Tibetan (PRC) - Updated"},
    {0xF02F, L"Khmer (NIDA)"},
    {0xF00E, L"Syriac Phonetic"},
    {0xF0A0, L"Sinhala - Wij 9"},
    {0xF037, L"Cherokee Nation Phonetic"},
    {0xF0A7, L"Inuktitut - Naqittaut"},
    {0xF00D, L"Divehi Typewriter"},
    {0xF0AD, L"Uyghur"},
    {0xF01E, L"Belgian (Comma)"},
    {0xF02D, L"Finnish with Sami"},
    {0xF0B2, L"Traditional Mongolian (Standard)"},
    {0xF030, L"Myanmar (Phonetic order)"},
    {0xF020, L"Canadian Multilingual Standard"},
    {0xF036, L"Tifinagh (Full)"},
    {0xF026, L"Gaelic"},
    {0xF029, L"Arabic (102) AZERTY"},
    {0xF0A3, L"Bulgarian (Phonetic)"},
    {0xF00A, L"Czech Programmers"},
    {0xF018, L"Greek (319)"},
    {0xF001, L"US (INTL)"},
    {0xF03D, L"Hebrew (Standard)"},
    {0xF0A6, L"Romanian (Programmers)"},
    {0xF033, L"Russian - Mnemonic"},
    {0xF022, L"Thai Kedmanee (non-ShiftLock)"},
    {0xF0A8, L"UA (Enhanced)"},
    {0xF0B1, L"Latvian (Standard)"},
    {0xF0A1, L"Lithuanian Standard"},
    {0xF038, L"Armenian Phonetic"},
    {0xF0AE, L"Sorbian Standard"},
    {0xF0AC, L"Georgian (Ergonomic)"},
    {0xF0A9, L"Bengali - INSCRIPT"},
    {0xF0C8, L"Tamil 99"},
    {0xF02E, L"Sami Extended Finland-Sweden"},
    {0xF031, L"New Tai Lue"},
    {0xF0AA, L"Bulgarian"},
    {0xF017, L"Greek (220) Latin"},
    {0xF01A, L"United States-Dvorak for left hand"},
    {0xF023, L"Thai Pattachote (non-ShiftLock)"},
    {0xF039, L"Armenian Typewriter"},
    {0xF03B, L"Georgian Ministry of Education and Science Schools"},
    {0xF034, L"Tai Le"},
    {0xF0AF, L"Bulgarian (Phonetic Traditional)"},
    {0xF011, L"Greek (319) Latin"},
    {0xF01B, L"United States-Dvorak for right hand"},
    {0xF03C, L"Georgian (Old Alphabets)"},
    {0xF032, L"Ogham"},
    {0xF019, L"Greek Latin"},
    {0xF00B, L"US English Table for IBM Arabic 238_L"},
    {0xF03A, L"Persian (Standard)"},
    {0xF01F, L"Greek Polytonic"},
    {0xF03E, L"Lisu (Basic)"},
    {0xF03F, L"Lisu (Standard)"},
    {0xF040, L"N’Ko"},
    {0xF041, L"Phags-pa"},
    {0xF0B3, L"Buginese"},
    {0xF0B4, L"Gothic"},
    {0xF0B6, L"Ol Chiki"},
    {0xF0B7, L"Osmanya"},
    {0xF0B8, L"Old Italic"},
    {0xF0B9, L"Sora"},
    {0xF0BA, L"Javanese"},
    {0xF0BB, L"Futhark"},
    {0xF0CB, L"Myanmar (Visual Order)"},
    {0xF0CC, L"ADLaM"},
    {0xF0CD, L"Osage"}
};
};
