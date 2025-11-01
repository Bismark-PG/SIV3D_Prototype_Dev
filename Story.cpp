/*==============================================================================

	Manage Story [Story.cpp]

	Author : Team Re:ing >> Bismark

	Note :

==============================================================================*/
#include "stdafx.h"
#include "Story.h"

const Array<Array<String>> Story_Manager::m_charGrid = {
	{ U"あ", U"い", U"う", U"え", U"お", U"が", U"ぎ", U"ぐ", U"げ", U"ご" },
	{ U"か", U"き", U"く", U"け", U"こ", U"ざ", U"じ", U"ず", U"ぜ", U"ぞ" },
	{ U"さ", U"し", U"す", U"せ", U"そ", U"だ", U"ぢ", U"づ", U"で", U"ど" },
	{ U"た", U"ち", U"つ", U"て", U"と", U"ば", U"び", U"ぶ", U"べ", U"ぼ" },
	{ U"な", U"に", U"ぬ", U"ね", U"の", U"ぱ", U"ぴ", U"ぷ", U"ぺ", U"ぽ" },
	{ U"は", U"ひ", U"ふ", U"へ", U"ほ", U"ぁ", U"ぃ", U"ぅ", U"ぇ", U"ぉ" },
	{ U"ま", U"み", U"む", U"め", U"も", U"っ", U"ゃ", U"ゅ", U"ょ", U"☆" },
	{ U"や", U"ゆ", U"よ", U" ",  U" ",  U" ",  U"→",  U"。", U"゛", U"゜" },
	{ U"ら", U"り", U"る", U"れ", U"ろ", U"ー", U" ",  U" ",  U" ",  U" " },
	{ U"わ", U"を", U"ん", U" ",  U" ",  U" ",  U" ",  U" ",  U"戻", U"決" }
};

Story_Manager::Story_Manager(PlayerStats& playerStats)
	: m_playerStats(playerStats),
	m_font(24),
	m_speakerFont(26, Typeface::Bold),
	m_promptFont(18),
	m_nameInputFont(24, Typeface::Bold),
	m_nameBufferFont(28, Typeface::Bold),
	m_protagonistTexture(U"Assets/Me.png"),
	m_textBox(Arg::center = Vec2(Scene::Center().x, Scene::Height() - 95), Scene::Width() * 0.95, 150),
	m_speakerBoxBase(m_textBox.x, m_textBox.y - m_textBox.h / 2 - 40, 0, 40),
	m_protagonistPos(Scene::Center().x - 200, Scene::Center().y + 50),
	m_npcSilhouette(Scene::Center().x + 200, Scene::Center().y + 50, 120),
	m_gridCursor(0, 0),
	m_nameDisplayBox(Arg::center = Scene::Center().movedBy(0, -180), 520, 80),
	m_charGridBox(Arg::center = Scene::Center().movedBy(0, 60), 520, 400)
{
}

void Story_Manager::StartStory(StoryID id)
{
	m_currentScript.clear();
	m_currentLineIndex = 0;
	m_isActive = true;
	m_state = StoryState::Talking;
	m_lastStoryID = id;

	m_nameInputBuffer.clear();
	m_gridCursor = { 0, 0 };

	switch (id)
	{
	case StoryID::Opening1:
		m_currentScript = {
			{m_narrationSpeaker, U"(終業のチャイム)"},
			{U"モブ１", U"この間また出たんだってー。"},
			{U"モブ２", U"なにが？"},
			{U"モブ１", U"口裂け女"},
			{U"モブ２", U"あー、最近よく聞くよね、そういう**都市伝説**。"},
			{U"モブ２", U"この間**花子さん**に遭遇したって子もいたなー。"},
			{U"モブ１", U"怖いから一緒に帰ろうよ。女"},
			{U"モブ２", U"...別に元から一緒じゃん？"},
			{m_scriptProtagonist, U"...。"},
			{m_scriptProtagonist, U"都市伝説か...。"}
		};
		break;

	case StoryID::Opening2:
		m_currentScript = {
			{m_scriptProtagonist, U"(都市伝説なんて現実に存在するわけないのに。)"},
			{m_scriptProtagonist, U"(仮にいたとしても、情報社会である今のご時世に写真やらニュースやらが出回らないはずがないだろ。)"},
			{m_scriptProtagonist, U"...ん？"},
			{m_scriptProtagonist, U"なんだこれ...？ゴミ...じゃなさそうだよな。"},
			{m_scriptProtagonist, U"え、なんか動いてる...。風で動いてるように見えてるだけ？"},
			{U"？？？", U"...。"},
			{m_scriptProtagonist, U"いやめっちゃこっち来てるな...。"},
			{m_scriptProtagonist, U"...。"},
			{m_scriptProtagonist, U"逃げよう！"},
			{U"？？？", U"...。"},
			{m_scriptProtagonist, U"(めっちゃ付いてくる！！)"},
		};
		break;

	case StoryID::Opening3:
		m_currentScript = {
			{m_scriptProtagonist, U"(やっと撒けた...何だったんだあの謎生物？は...。)"},
			{m_scriptProtagonist, U"...？"},
			{m_scriptProtagonist, U"(何だろう、この女の人...雰囲気が...。)"},
			{U"マスク女", U"ねぇ、私、綺麗？"},
			{m_scriptProtagonist, U"(この質問、聞き覚えが...。)"},
			{m_scriptProtagonist, U"...はい、綺麗...です。"},
			{U"口裂け女", U"こ　れ　で　も　？"},
		};
		break;

	case StoryID::Tutorial_Name:
		m_currentScript = {
			{m_scriptProtagonist, U"ヤバい　ヤバい　ヤバい"},
			{m_scriptProtagonist, U"口裂け女！実在していたなんて！"},
			{m_scriptProtagonist, U"このまま捕まったら殺される！"},
			{U"口裂け女", U"どうして逃げるの？　私綺麗？　ねぇ、私綺麗？"},
			{m_scriptProtagonist, U"(あの謎生物！また何なんだよ！)"},
			{m_scriptProtagonist, U"くそ、一か八かだ！突っ切る！"},
			{m_scriptProtagonist, U"(！？コイツ、頭の上に...！)"},
			{U"？？？", U"(こんにちは、人間様。今から脳内に色々な情報を流し込むので頑張って耐えて下さい。それでは参ります。)"},
			{m_scriptProtagonist, U"(コイツ、直接脳内に...って、今なんて！？)"},
			{U"ナレーション", U"数多くの情報が脳内に直接流入する。"},
			{m_scriptProtagonist, U"っ...痛ェ...！"},
			{U"？？？", U"(耐えましたね、それではまず私に名付けをお願いします。)"},
			{m_scriptProtagonist, U"絶対に逃げる時にする事じゃないだろ！！"},
			{U"？？？", U"(...？情報を与えたのに伝わっていない...？)"},
			{m_scriptProtagonist, U"そっちじゃない！情報与える事だよ！！"},
			{U"？？？", U"(成程、それでは早急に私への名付けを。)"},
			{m_scriptProtagonist, U"調子狂うな...じゃあお前の名前は..."},
			{m_inputPartnerTag, U"パートナーの名前を入力"}
		};
		break;

	case StoryID::Tutorial_Battle:
		m_currentScript = {
			{m_scriptProtagonist, U"お前の名前は [name] だ！"},
			{U"[name]", U"(承認しました。それでは戦闘を開始します。)"},
			{U"[name]", U"(分かっていた事ですがやはり勝率は現在限りなくゼロに近いですね。)"},
			{U"[name]", U"(人間様、例の言葉を。)"},
			{m_scriptProtagonist, U"分かってたなら最初からで良かったんじゃないか...？"},
			{U"[name]", U"(私は子供のようなものなので、自分がどの程度通用するのか試したかったのですよ、人間様。)"},
			{m_scriptProtagonist, U"お前みたいな喋り方をする子供はいない。"},
		};
		break;

	case StoryID::Tutorial_Introduce:
		m_currentScript = {
			{U"[name]", U"(口裂け女を撃退しました。戦闘を終了します。)"},
			{U"[name]", U"(流石の手腕でしたね、人間様。あの情報に耐えただけあります。)"},
			{m_scriptProtagonist, U"それ、関係あるのか...？"},
			{m_scriptProtagonist, U"それはそれとして、その『人間様』ってのやめてくれ。"},
			{U"[name]", U"(『様』は自分より立場が上の方に付ける呼称ではないのですか？)"},
			{m_scriptProtagonist, U"さっき名付けたばっかりだけど、お前に [name] って名前があるように、僕にも名前があるんだよ。"},
			{U"[name]", U"((成程。では何と呼べば？)"},
			{m_inputPlayerTag, U"あなたの名前を入力"}
		};
		break;

	case StoryID::Tutorial_End:
		m_currentScript = {
		{U"[player]", U"僕の名前は [player]だ。"},
		};
		break;

	case StoryID::Day2_Start:
		m_currentScript = {
			{m_scriptProtagonist, U"벌써 2일차인가... 시간이 얼마 없어."},
			{U"？？？", U"서두르는 게 좋을 거다."}
		};
		break;

	case StoryID::FinalBattle_Start:
		m_currentScript = {
			{U"？？？", U"드디어 마지막 날이군."},
			{m_scriptProtagonist, U"당신은...!"},
			{U"？？？", U"모든 것을 끝내자."}
		};
		break;

	case StoryID::Ending_Clear:
		m_currentScript = {
			{m_scriptProtagonist, U"끝났다..."},
			{U"？？？", U"해냈군. 하지만 이건 시작일 뿐이다."},
			{m_scriptProtagonist, U"(The End)"}
		};
		break;
	}
}

bool Story_Manager::Update()
{
	if (not m_isActive)
	{
		return false;
	}

	if (m_state == StoryState::Talking)
	{
		if (KeyZ.down())
		{
			m_currentLineIndex++;
			if (m_currentLineIndex >= m_currentScript.size())
			{
				m_isActive = false;
				return true;
			}

			const auto& nextLine = m_currentScript[m_currentLineIndex];
			if (nextLine.speaker == m_inputPartnerTag)
			{
				m_state = StoryState::NamingPartner;
				m_nameInputBuffer.clear();
				m_gridCursor = { 0, 0 };
			}
			else if (nextLine.speaker == m_inputPlayerTag)
			{
				m_state = StoryState::NamingPlayer;
				m_nameInputBuffer.clear();
				m_gridCursor = { 0, 0 };
			}
		}
	}
	else
	{
		if (KeyUp.down())
		{
			m_gridCursor.y = (m_gridCursor.y - 1 + m_gridRows) % m_gridRows;
		}
		if (KeyDown.down())
		{
			m_gridCursor.y = (m_gridCursor.y + 1) % m_gridRows;
		}
		if (KeyLeft.down())
		{
			m_gridCursor.x = (m_gridCursor.x - 1 + m_gridCols) % m_gridCols;
		}
		if (KeyRight.down())
		{
			m_gridCursor.x = (m_gridCursor.x + 1) % m_gridCols;
		}

		if (KeyX.down())
		{
			if (not m_nameInputBuffer.isEmpty())
			{
				m_nameInputBuffer.pop_back();
			}
		}

		if (KeyZ.down())
		{
			const String& selectedChar = m_charGrid[m_gridCursor.y][m_gridCursor.x];

			if (selectedChar == U"決")
			{
				if (m_nameInputBuffer.length() >= 2 && m_nameInputBuffer.length() <= m_maxNameLength)
				{
					if (m_state == StoryState::NamingPartner)
					{
						m_playerStats.partnerName = m_nameInputBuffer;
						m_state = StoryState::Talking;
						StartStory(StoryID::Tutorial_Battle);
					}
					else if (m_state == StoryState::NamingPlayer)
					{
						m_playerName = m_nameInputBuffer;
						m_state = StoryState::Talking;

						m_currentLineIndex++;
						if (m_currentLineIndex >= m_currentScript.size())
						{
							m_isActive = false;
							return true;
						}
					}
				}
				else
				{
					Console << U"Debug : 名前は2~8文字でなければなりません";
				}
			}
			else if (selectedChar == U"戻")
			{
				if (not m_nameInputBuffer.isEmpty())
				{
					m_nameInputBuffer.pop_back();
				}
			}
			else if (selectedChar != U" ") 
			{
				if (m_nameInputBuffer.length() < m_maxNameLength)
				{
					m_nameInputBuffer += selectedChar;
				}
			}
		}
	}

	return false;
}

void Story_Manager::Draw() const
{
	if (not m_isActive)
	{
		return;
	}

	if (m_state == StoryState::Talking)
	{
		RectF(Scene::Size()).draw(ColorF(0.0, 0.0, 0.0, 0.5));
	}

	if (m_currentLineIndex >= m_currentScript.size())
	{
		return;
	}

	const auto& line = m_currentScript[m_currentLineIndex];

	String speakerToShow = line.speaker;
	String textToDraw = line.text;

	if (not m_playerStats.partnerName.isEmpty())
	{
		speakerToShow.replace(U"[name]", m_playerStats.partnerName);
		textToDraw.replace(U"[name]", m_playerStats.partnerName);
	}
	speakerToShow.replace(m_protagonistSpeaker, m_playerName);
	textToDraw.replace(m_protagonistSpeaker, m_playerName);
	speakerToShow.replace(m_scriptProtagonist, m_playerName);
	textToDraw.replace(m_scriptProtagonist, m_playerName);
	speakerToShow.replace(U"[player]", m_playerName);
	textToDraw.replace(U"[player]", m_playerName);


	const ColorF protagonistColor = (speakerToShow == m_playerName)
		? ColorF(1.0)
		: ColorF(0.5, 0.7);
	const ColorF npcColor = (speakerToShow != m_playerName && speakerToShow != m_narrationSpeaker)
		? ColorF(1.0, 0.6, 0.5, 0.8)
		: ColorF(0.5, 0.7);

	m_protagonistTexture
		.scaled(0.8)
		.drawAt(m_protagonistPos, protagonistColor);
	m_npcSilhouette.draw(npcColor).drawFrame(2, Palette::Black);

	m_textBox.draw(ColorF(0.1, 0.1, 0.2, 0.9)).drawFrame(2, Palette::White);
	m_font(textToDraw).draw(m_textBox.x + 30, m_textBox.y + 30, Palette::White);

	if (m_state == StoryState::Talking)
	{
		m_promptFont(U"次へ : Z").draw(Arg::bottomRight(m_textBox.br().movedBy(-20, -15)), Palette::White);
	}

	if (speakerToShow != m_narrationSpeaker)
	{
		RectF speakerRect = m_speakerBoxBase;
		speakerRect.w = m_speakerFont(speakerToShow).region().w + 30;
		speakerRect.draw(ColorF(0.4, 0.3, 0.8, 0.9)).drawFrame(2, Palette::White);
		m_speakerFont(speakerToShow).drawAt(speakerRect.center(), Palette::White);
	}


	if (m_state == StoryState::NamingPartner || m_state == StoryState::NamingPlayer)
	{
		DrawNamingUI(line.text);
	}
}

void Story_Manager::DrawNamingUI(const String& titleText) const
{
	RectF(Scene::Size()).draw(ColorF(0.0, 0.0, 0.0, 0.7));

	m_nameDisplayBox.draw(ColorF(0.1, 0.1, 0.1, 0.9)).drawFrame(2, Palette::White);
	m_nameBufferFont(m_nameInputBuffer).drawAt(m_nameDisplayBox.center(), Palette::White);
	m_font(U"{}/{}"_fmt(m_nameInputBuffer.length(), m_maxNameLength)).draw(Arg::bottomRight(m_nameDisplayBox.br().movedBy(-15, -10)), Palette::Gray);
	m_font(titleText).draw(Arg::topLeft(m_nameDisplayBox.tl().movedBy(15, -30)), Palette::White);

	m_charGridBox.draw(ColorF(0.1, 0.1, 0.1, 0.9)).drawFrame(2, Palette::White);

	const double cellWidth = m_charGridBox.w / m_gridCols;
	const double cellHeight = m_charGridBox.h / m_gridRows;

	for (int y = 0; y < m_gridRows; ++y)
		for (int x = 0; x < m_gridCols; ++x)
		{
			const RectF cell(m_charGridBox.x + x * cellWidth, m_charGridBox.y + y * cellHeight, cellWidth, cellHeight);
			const String& str = m_charGrid[y][x];

			if (str == U" ") continue;

			if (str == U"決" || str == U"戻")
			{
				cell.stretched(-2).draw(ColorF(0.4, 0.4, 0.4));
				m_nameInputFont(str).drawAt(cell.center(), Palette::White);
			}
			else
			{
				m_nameInputFont(str).drawAt(cell.center(), Palette::White);
			}

			if (Point{ x, y } == m_gridCursor)
			{
				cell.drawFrame(3.0, 0.0, Palette::Red);
			}
		}
}

bool Story_Manager::IsActive() const
{
	return m_isActive;
}

StoryID Story_Manager::GetLastStoryID() const
{
	return m_lastStoryID;
}
