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
	m_protagonistTexture(TextureAsset(U"Player")),
	m_textBox(Arg::center = Vec2(Scene::Center().x, Scene::Height() - 95), Scene::Width() * 0.95, 150),
	m_speakerBoxBase(m_textBox.x, Scene::Center().y + 80, 0, 40),
	m_protagonistPos(Scene::Center().x + 200, Scene::Center().y + 50),
	m_aibo_Pos(Scene::Center().x + 150, Scene::Center().y),
	m_gridCursor(0, 0),
	m_nameDisplayBox(Arg::center = Scene::Center().movedBy(0, -180), 520, 80),
	m_charGridBox(Arg::center = Scene::Center().movedBy(0, 60), 520, 400)
{
	m_aibo_Anime.emplace(U"../Assets/BattleCharacter/BattlePlayer.png", 96, 128, 7, 1, 0.0, 128.0, 0.0, 0.0);
	if (m_aibo_Anime)
	{
		m_aibo_Anime->BattleAnim_SetFPS(8);
		m_aibo_Anime->BattleAnim_SetLoop(true);
		m_aibo_Anime->BattleAnim_SetScale(1.0);
		m_aibo_Anime->BattleAnim_SetPos(m_aibo_Pos);
		m_aibo_Anime->BattleAnim_Start(true);
	}
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
		// BGM : No
	case StoryID::Story_For_Skip:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentScript = {
		{m_narrationSpeaker, U"このゲームは、7日間の戦闘と探索で成長リソースを集め、\n自分だけの都市伝説を育てるゲームです。"},
		{m_narrationSpeaker, U"8日目になった瞬間、強力な敵と戦闘をしなければならないので、\n準備をしていきましょう。"},
		{m_narrationSpeaker, U"好きなルートで自分だけの都市伝説を成長させてみます。"},
		};
		break;

	case StoryID::Skip_Input_Player_Name:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"";
		m_currentScript = {
		{m_inputForSkip, U"あなたの名前を入力(2~8文)"}
		};
		m_state = StoryState::NamingPlayer;
		break;

	case StoryID::Skip_Input_Partner_Name:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"";
		m_currentScript = {
		{m_inputForSkip, U"パートナーの名前を入力(2~8文)"}
		};
		m_state = StoryState::NamingPartner;
		break;

		// BGM : Explorer
	case StoryID::Opening1:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Explorer";
		m_currentScript = {
			{m_narrationSpeaker, U"(終業のチャイム)"},
			{m_mob1, U"この間また出たんだってー。"},
			{m_mob2, U"なにが？"},
			{m_mob1, U"[口裂け女]"},
			{m_mob2, U"あー、最近よく聞くよね、そういう [都市伝説]。"},
			{m_mob2, U"この間 [花子さん] に遭遇したって子もいたなー。"},
			{m_mob1, U"怖いから一緒に帰ろうよ。女"},
			{m_mob2, U"...別に元から一緒じゃん？"},
			{m_scriptProtagonist, U"...。"},
			{m_scriptProtagonist, U"都市伝説か...。"}
		};
		break;

		// BGM : Scary
	case StoryID::Opening2:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Scary";
		m_currentScript = {
			{m_scriptProtagonist, U"(都市伝説なんて現実に存在するわけないのに。)"},
			{m_scriptProtagonist, U"(仮にいたとしても、情報社会である今のご時世に写真やら\n ニュースやらが出回らないはずがないだろ。)"},
			{m_scriptProtagonist, U"...ん？"},
			{m_scriptProtagonist, U"なんだこれ...？ゴミ...じゃなさそうだよな。"},
			{m_scriptProtagonist, U"え、なんか動いてる...。\n 風で動いてるように見えてるだけ？"},
			{m_playerStats.partnerName, U"...。"},
			{m_scriptProtagonist, U"いやめっちゃこっち来てるな...。"},
			{m_scriptProtagonist, U"...。"},
			{m_scriptProtagonist, U"逃げよう！"},
			{m_playerStats.partnerName, U"...。"},
			{m_scriptProtagonist, U"(めっちゃ付いてくる！！)"},
		};
		break;

		// BGM : Scary
	case StoryID::Opening3:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Scary";
		m_currentScript = {
			{m_scriptProtagonist, U"(やっと撒けた...何だったんだあの謎生物？は...。)"},
			{m_scriptProtagonist, U"...？"},
			{m_scriptProtagonist, U"(何だろう、この女の人...雰囲気が...。)"},
			{m_mob3, U"ねぇ、私、綺麗？"},
			{m_scriptProtagonist, U"(この質問、聞き覚えが...。)"},
			{m_scriptProtagonist, U"...はい、綺麗...です。"},
			{m_mob4, U"こ　れ　で　も　？"},
		};
		break;

		// BGM : Chase
	case StoryID::Tutorial_Name:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Chase";
		m_currentScript = {
			{m_scriptProtagonist, U"ヤバい　ヤバい　ヤバい"},
			{m_scriptProtagonist, U"[口裂け女]！実在していたなんて！"},
			{m_scriptProtagonist, U"このまま捕まったら殺される！"},
			{m_mob4, U"どうして逃げるの？　私綺麗？　ねぇ、私綺麗？"},
			{m_scriptProtagonist, U"(あの謎生物！また何なんだよ！)"},
			{m_scriptProtagonist, U"くそ、一か八かだ！突っ切る！"},
			{m_scriptProtagonist, U"(！？コイツ、頭の上に...！)"},
			{m_playerStats.partnerName, U"(こんにちは、人間様。\n 今から脳内に色々な情報を流し込むので頑張って耐えて下さい。それでは参ります。)"},
			{m_scriptProtagonist, U"(コイツ、直接脳内に...って、今なんて！？)"},
			{m_narrationSpeaker, U"数多くの情報が脳内に直接流入した。"},
			{m_scriptProtagonist, U"っ...痛ェ...！"},
			{m_playerStats.partnerName, U"(耐えましたね、それではまず私に名付けをお願いします。)"},
			{m_scriptProtagonist, U"絶対に逃げる時にする事じゃないだろ！！"},
			{m_playerStats.partnerName, U"(...？情報を与えたのに伝わっていない...？)"},
			{m_scriptProtagonist, U"そっちじゃない！情報与える事だよ！！"},
			{m_playerStats.partnerName, U"(成程、それでは早急に私への名付けを。)"},
			{m_scriptProtagonist, U"調子狂うな...じゃあお前の名前は..."},
			{m_inputPartnerTag, U"パートナーの名前を入力(2~8文)"}
		};
		break;

		// BGM : Normal_Battle
	case StoryID::Tutorial_Battle:
		m_currentRenderType = StoryRenderType::Overlay;
		m_currentBGM = U"Battle";
		m_currentScript = {
			{m_scriptProtagonist, U"お前の名前は 「[name]」だ！"},
			{U"[name]", U"(承認しました。それでは戦闘を開始します。)"},
			{U"[name]", U"(分かっていた事ですがやはり勝率は現在限りなくゼロに近いですね。)"},
			{U"[name]", U"(人間様、例の言葉を。)"},
			{m_scriptProtagonist, U"分かってたなら最初からで良かったんじゃないか...？"},
			{U"[name]", U"(私は子供のようなものなので、自分がどの程度通用するのか試したかったのですよ、人間様。)"},
			{m_scriptProtagonist, U"お前みたいな喋り方をする子供はいない。"},
		};
		break;

		// BGM : 歪む霊域
	case StoryID::Tutorial_Introduce:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"歪む霊域";
		m_currentScript = {
			{U"[name]", U"(口裂け女を撃退しました。戦闘を終了します。)"},
			{U"[name]", U"(流石の手腕でしたね、人間様。あの情報に耐えただけあります。)"},
			{m_scriptProtagonist, U"それ、関係あるのか...？"},
			{m_scriptProtagonist, U"それはそれとして、その『人間様』ってのやめてくれ。"},
			{U"[name]", U"(『様』は自分より立場が上の方に付ける呼称ではないのですか？)"},
			{m_scriptProtagonist, U"さっき名付けたばっかりだけど、お前に 「[name]」って名前があるように、僕にも名前があるんだよ。"},
			{U"[name]", U"((成程。では何と呼べば？)"},
			{m_inputPlayerTag, U"あなたの名前を入力(2~8文)"}
		};
		break;

		// BGM : 歪む霊域
	case StoryID::Tutorial_End:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"歪む霊域";
		m_currentScript = {
		{U"[player]", U"僕の名前は 「[player]」だ。"},
		};
		break;

		// BGM : 歪む霊域
	case StoryID::Day1_End:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"禁止区";
		m_background = TextureAsset(U"Grow_Back");
		m_currentScript = {
		{U"[player]", U"なぁ、[name]。"},
		{U"[name]", U"(何でしょうか、「[player]」様。)"},
		{U"[player]", U"『様』はいらないよ。というか脳に直接喋るの慣れないんだけど普通に話せない？"},
		{U"[name]", U"([player]がそう思えば口と発声器官が創られると思いますよ。)"},
		{U"[name]", U"(それと『普通に』はちょっと傷つきますよ。私のこれまでの『普通』はこれなんですから。)"},
		{U"[name]", U"(今の時代は多様性ですよ。都市伝説にも適応して下さい。)"},
		{U"[player]", U"なんかごめん。"},
		{U"[name]", U"それで何でしょうか、[player]。"},
		{U"[player]", U"さっき僕はお前にすごい量の情報を流し込まれたわけじゃん。"},
		{U"[name]", U"そうですね。"},
		{U"[player]", U"あれ、理解の有無問わずに無理やり流し込んでるよね？"},
		{U"[name]", U"まぁ、はい。"},
		{U"[player]", U"(なんでコイツちょっと不貞腐れてるんだ...)"},
		{U"[player]", U"その情報に僕とお前で齟齬があったらいけないだろ、だからさっきのおさらいをさせてくれ。"},
		{U"[name]", U"成程、多すぎる情報による弊害その②ですね。御意に。"},
		// --- (アルティメット説明タイム) --- //
		{U"[player]", U"そもそもの話、[都市伝説はこの世に実在する] んだな？"},
		{U"[name]", U"はい。先程目にした口裂け女や私がその最たる証拠ですが、実在するかどうかで言うとグレーですね。"},
		{U"[name]", U"言うならば我々は『真になった嘘』ですので。"},
		{U"[player]", U"噂で出来た情報の集合体、か。"},
		{U"[name]", U"はい。我々は人などから出される噂によって形を成し、力を手に入れます。"},
		{U"[name]", U"その都市伝説の中の１体に、私たちは監禁されています。"},
		{U"[name]", U"私はその都市伝説を倒そうと思い、貴方に協力を仰ぎました。"},
		{U"[player]", U"色々訊きたい事はあるけど...まずなんで僕の助けがいるんだ？"},
		{U"[name]", U"それは『何故私が助けを必要としているか』でしょうか。『何故人間に助けを求めたか』でしょうか。"},
		{U"[name]", U"それとも、『何故協力者に [player] を選んだか』？"},
		{U"[player]", U"まぁ大体全部かな...先２つはざっくりわかるけど...。"},
		{U"[name]", U"それでは簡潔に。"},
		{U"[name]", U"①私が都市伝説として未熟だからです。都市伝説ではあるものの、まだ何者でもない、中途半端な個体なので。"},
		{U"[name]", U"②都市伝説を育てるには人間が最適だからです。現代社会に於いては誰もが片手間で噂を取り入れ、そして発信する事が出来るようになってしまったので。"},
		{U"[name]", U"③たまたま近くにいたあの情報に耐えうる人間が[player]だったので。"},
		{U"[player]", U"たまたまかよ...その情報に耐えうるってのは何を以て判断してるんだ？"},
		{U"[name]", U"与える情報の受け入れやすさですね。[player]は、都市伝説がお好きで？"},
		{U"[player]", U"まぁ動画とかで結構見たりはするけど...それって見て判断できるものなのか？それとも都市伝説の能力？"},
		{U"[name]", U"...？分かりませんよ、そんなことは。[player]が耐えうる人間じゃなければ別な人間を探していました。"},
		{U"[player]", U"...もし情報に耐えられなかった場合はどうなるんだ？"},
		{U"[name]", U"廃人になるでしょうね。"},
		{U"[player]", U"お前...人間を使い捨ての駒みたいに...！"},
		{U"[name]", U"先程も申し上げましたが、貴方(人間)の価値観を押し付けないで下さい。この世界の解放の重大さに比べればそんなことは些事でしかないですよ。"},
		{U"[player]", U"人の人生が些事であっていい訳がないだろ！"},
		{U"[name]", U"それにこの世界は解放された後恐らく攫われる前に時間に回帰しますよ。それでも貴方からすれば憤慨するようなことなのでしょうか？"},
		{U"[player]", U"そういうことじゃあ..."},
		{U"[player]", U"いや、もういい。"},
		{U"[name]", U"理解していただけましたか。"},
		{U"[player]", U"お前達とは相容れないってことがね。"},
		{U"[player]", U"僕達の情報から出来たとはいえ、お前達は情報の上辺だけを切り取って存在しているんだ。"},
		{U"[player]", U"だから決して理解することも無いし、されることも無いと思うよ。"},
		{U"[name]", U"左様ですか。私としてもお友達ごっこをするつもりなど毛頭ありません。"},
		{U"[name]", U"そも回帰されれば無かったことになりますので、元から７日のみの交流だと思っています。"},
		{U"[player]", U"そうか。"},
		{U"[player]", U"ところでその７日てのはどこから出てきた数字だ？"},
		{U"[name]", U"奴と戦う期限です。それ以降、奴は大成して勝算はほぼゼロに等しくなるでしょう。"},
		{U"[player]", U"１週間てのは大分短いが...その僕達を閉じ込めている都市伝説についての情報の仔細が欲しい。"},
		{U"[name]", U"はい"},
		{U"[name]", U"その都市伝説の名は『きさらぎ』。かつて『きさらぎ駅』という名で存在していた都市伝説です。"},
		{U"[name]", U"近代日本においてあまりにも名のある都市伝説でしたが、それは大きくなりすぎました。"},
		{U"[name]", U"他の類似点を持つ都市伝説の吸収を繰り返し、あまつさえ都市そのものとなった、正しく『都市』伝説です。"},
		{U"[name]", U"それは人や動物、都市伝説を我が身の中に誘い、自分の分け身を這わせ、噂を流させることによって、現在進行形で強くなり続けています。"},
		{U"[player]", U"なんて厄介な...場所系の都市伝説だしこれといって弱点も思いつかないけど...。"},
		{U"[player]", U"戦うってことは勝てる見込みはあるんだろ？"},
		{U"[name]", U"私と[player]の努力次第ですね。"},
		{U"[name]", U"私は今は未熟ではありますが、裏を返せば何者にも成れることができます。"},
		{U"[player]", U"成程な。それで今僕に出来る事は？"},
		{U"[name]", U"ありません。寝て下さい。"},
		{U"[player]", U"あ、はい。おやすみ。"},
		};
		break;

		// BGM : Explorer
	case StoryID::Day2_Start:
		m_currentRenderType = StoryRenderType::Overlay;
		m_currentBGM = U"Explorer";
		m_currentScript = {
		{U"[player]", U"で、何をすればいい？"},
		{U"[name]", U"簡単に言うなら、『探索』と『戦闘』です。"},
		{U"[name]", U"『探索』では、私の噂を広める事で私の強化ができたり、有効的なアイテムを入手することができるかもしれません。"},
		{U"[name]", U"『戦闘』では、他の都市伝説と戦闘をして勝つことで、都市伝説達に私の噂が広まり強化に繋がります。"},
		{U"[name]", U"他の都市伝説の狂化の恐れがあるので丑三つ時前には帰りましょう。"},
		{U"[player]", U"帰ったらどんな都市伝説に育成するか考えないとな。"},
		};
		break;

		// BGM : 禁止区
	case StoryID::FinalBattle_Start:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"禁止区";
		m_currentScript = {
		{U"[player]", U"もう7日目か...。長いようで短かったな。"},
		{U"[name]", U"勝てる確率は５割前後、といったところですね。"},
		{U"[name]", U"それでは参りましょうか。"},
		{U"[player]", U"...。"},
		{U"[player]", U"なぁ[name]、お前はどうしてここから出ようと思ったんだ？"},
		{U"[name]", U"...？なんですか、藪から棒に。"},
		{U"[player]", U"いや、最初の方に訊こうと思ってたんだけど訊くタイミング逃しちゃってさ。"},
		{U"[player]", U"今がゆっくり話せる最後の時間かもだろ？"},
		{U"[name]", U"そうですね...強いて言うのならここに居るべきではないと思ったからです。"},
		{U"[name]", U"朧げではありますが、私を形成していた数少ない情報の中に、場所にまつわるものがあったのだと思います。"},
		{U"[player]", U"花子さんがトイレに縛られるように、[name]もどこかの場所に縛られる都市伝説だったってことか。"},
		{U"[name]", U"合理的な事ではないと自分でも分かっています。"},
		{U"[name]", U"このまま世界全てが『きさらぎ』の一部となったとしても、\n 我々都市伝説にとってはデメリットなど無いに等しいのですから。"},
		{U"[name]", U"それでも、"},
		{m_narrationSpeaker, U"魂が否定した\n 情報否定しました。"},
		{U"[player]", U"なんだ、すごい良いじゃん。"},
		{U"[player]", U"人間臭くて。"},
		{U"[name]", U"人間臭い...私がですか？"},
		{U"[player]", U"あぁ。それがたとえどんなに合理的であっても、\n 魂が否定するから従わない。"},
		{U"[player]", U"勉強だけを続けていればいい？\n 嫌だ。遊ぶ事が楽しいから。"},
		{U"[player]", U"完全栄養食だけさえ食べていればいい？\n 嫌だ。美味しいものを味わうことが人間の美徳だからだ。"},
		{U"[player]", U"親が選ぶ学校に言って親が決めた職に就け？\n 嫌だ。僕には夢があるからだ。"},
		{U"[player]", U"夢が、志が、心の豊かさこそ人の素晴らしさで、\n 僕が人間らしさを好きな理由だ。"},
		{U"[player]", U"それは[name]でも変わらないよ。"},
		{U"[name]", U"人間らしさ...成程、情報(こころ)にしかと刻んでおきます。"},
		{U"[player]", U"うん。記憶が消えたとしても、心に刻んだものは、そいつの在り方として変わることはないよ。僕も、[name]も。"},
		{U"[player]", U"それじゃあ行こうか、この長い都市伝説の結末を決めに。"},
		};
		break;

		// BGM : Ending_Bad
	case StoryID::Ending_Bad:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Ending_Bad";
		m_currentScript = {
		{U"[name]", U"申し訳ありません、[player]。"},
		{U"[name]", U"私の力が及ばなかったようです。"},
		{U"[player]", U"いいや、謝らないでくれ、[name]。"},
		{U"[player]", U"僕も、自分は世界を救える勇者になれると驕っていたみたいだ。"},
		{U"[player]", U"それでも、潔く死ぬなんてまっぴらごめんだ。"},

		{m_narrationSpeaker, U"([player]が[name]を投げ飛ばした)"},
		{U"[name]", U"[player]...！？何を...。"},
		{U"[player]", U"次の協力者を探せ！　[name]！"},
		{U"[player]", U"たとえコイツが大成したとしても、勝機はゼロではないはずだ！"},
		{U"[player]", U"たとえ僕が、"},
		};

		// BGM : Ending_Good
	case StoryID::Ending_Good:
		m_currentRenderType = StoryRenderType::Standalone;
		m_currentBGM = U"Ending_Good";
		m_currentScript = {
		{U"[name]", U"きさらぎを撃退しました。　戦闘を終了します。"},
		{U"[player]", U"...！空が崩れ落ちていく...。"},
		{U"[name]", U"きさらぎを倒したことにより、この街の瓦解が始まったようですね。"},
		{U"[name]", U"もう暫くしたら、私達は攫われる前の状態に回帰するでしょうね。場所も、記憶も。"},
		{U"[player]", U"[name]ともここでお別れか。"},
		{U"[player]", U"ただの協力関係だったけど、別れとなると流石に名残惜しいな。"},
		{U"[name]", U"...そうですね。"},
		{U"[name]", U"[player]。"},
		{U"[name]", U"もしかしたら今生の別れになるかもしれないので、改めて貴方に感謝を。"},
		{U"[player]", U"そうだな、もう出会えるか分からないし...。"},
		{U"[name]", U"いえ、もし回帰したら私は消滅、ないしは死ぬ可能性がおおいにありますので。"},
		{U"[player]", U"っ...！？どういうことだよ、それ...！？"},
		{U"[name]", U"私は所詮都市伝説のなりそこないです。"},
		{U"[name]", U"今でこそ[player]に協力してもらい、ここまで育つことはできましたが、あの時点での私は、本当に運よく存在を保っていただけですので。"},
		{U"[player]", U"僕みたいに戻ったら協力者を作る事はできないのか？"},
		{U"[name]", U"できない...というより、あの頃の私はやらない...が正しいですね。"},
		{U"[name]", U"消えるなら消えるべきものだった、そう思うでしょうね。"},
		{U"[name]", U"なので今のうちに..."},
		{U"[player]", U"いや、さよならは言わない。"},
		{U"[player]", U"戦う前に言ったじゃないか。"},
		{U"[player]", U"『記憶が消えたとしても、心に刻んだものは、在り方として変わらない』って。"},
		{U"[player]", U"だから、回帰したとしても、[name]はあの頃の[name]よりも少し人間らしくなって、しぶとく生きるって、僕は信じてる。"},
		{U"[name]", U"[player]...。"},
		{U"[player]", U"それに、僕が[name]を見つけてみせるよ。"},
		{U"[player]", U"だから、[name]も僕を探して欲しい。"},
		{U"[name]", U"...何年、何十年かかっても文句は受け付けませんよ。"},
		{U"[player]", U"こちらこそ。"},
		{U"[player]", U"次会った時は、何かの協力者ではなく、友達として。"},
		{U"[name]", U"友達...良いですね。人間臭くて好きですよ。"},
		{U"[name]", U"...そろそろ崩壊が完全に終わるようですね。"},
		{U"[player]", U"じゃあ別れの挨拶をば...。"},
		{U"[name]", U"さよならは言わない、じゃなかったんですか？"},
		{U"[player]", U"『さよなら』は言わない。次また会えると信じてるから。"},
		{U"[player]", U"それじゃあ[name]、"},
		{U"[player]", U"またな！"},
		{U"[name]", U"...はい！またいつか！"},
		};
		break;
	}
}

bool Story_Manager::Update()
{
	if (m_showNameWarning)
	{
		if (KeyZ.down())
		{
			m_showNameWarning = false;
		}
		return false;
	}

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
				StoryID finishedStory = GetLastStoryID();

				if (finishedStory == StoryID::Ending_Bad || finishedStory == StoryID::Ending_Good)
				{
					if (not m_currentBGM.isEmpty())
					{
						AudioAsset(m_currentBGM).stop(SecondsF(0.5));
					}
				}
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
			m_gridCursor = FindNextValidCursorPos(m_gridCursor, 0, -1);
		}
		if (KeyDown.down())
		{
			m_gridCursor = FindNextValidCursorPos(m_gridCursor, 0, 1);
		}
		if (KeyLeft.down())
		{
			m_gridCursor = FindNextValidCursorPos(m_gridCursor, -1, 0);
		}
		if (KeyRight.down())
		{
			m_gridCursor = FindNextValidCursorPos(m_gridCursor, 1, 0);
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
						if (m_nameInputBuffer == m_playerName)
						{
							Console << U"Debug : Same name to aido";
							m_showNameWarning = true;
						}
						else
						{
							m_playerStats.partnerName = m_nameInputBuffer;
							m_state = StoryState::Talking;
							m_currentLineIndex++;
							if (m_currentLineIndex >= m_currentScript.size())
							{
								m_isActive = false;
								return true;
							}
						}
					}
					else if (m_state == StoryState::NamingPlayer)
					{
						if (m_nameInputBuffer == m_playerStats.partnerName)
						{
							Console << U"Debug : Same name to player";
							m_showNameWarning = true;
						}
						else
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

	if (m_aibo_Anime)
	{
		m_aibo_Anime->BattleAnim_Update();
	}
	return false;
}

void Story_Manager::Draw() const
{
	if (not m_isActive)
	{
		return;
	}

	if (m_currentRenderType == StoryRenderType::Standalone)
	{
		if (m_background)
		{
			m_background.scaled(Scene::Width() / (double)m_background.width()).draw(0, 0);
		}
		else
		{
			Scene::Rect().draw(ColorF(0.0));
		}
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


	if (speakerToShow == m_playerName || speakerToShow == m_scriptProtagonist)
	{
		m_protagonistTexture
			.scaled(0.8)
			.drawAt(m_protagonistPos, ColorF(1.0));
	}
	else if (speakerToShow != m_narrationSpeaker
		&& speakerToShow != m_mob1
		&& speakerToShow != m_mob2
		&& speakerToShow != m_mob3
		&& speakerToShow != m_mob4
		&& speakerToShow != m_inputForSkip)
	{
		if (m_aibo_Anime)
		{
			m_aibo_Anime->BattleAnim_Draw();
		}
	}

	m_textBox.draw(ColorF(0.1, 0.1, 0.2, 0.9)).drawFrame(2, Palette::White);
	m_font(textToDraw).draw(m_textBox.x + 30, m_textBox.y + 30, Palette::White);

	if (m_state == StoryState::Talking)
	{
		m_promptFont(U"次へ : Z").draw(Arg::bottomRight(m_textBox.br().movedBy(-20, -15)), Palette::White);
	}

	if (speakerToShow != m_narrationSpeaker
		&& speakerToShow != m_inputPartnerTag
		&& speakerToShow != m_inputPlayerTag
		&& speakerToShow != m_inputForSkip)
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

	if (m_showNameWarning)
	{
		DrawDuplicateNameWarning();
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

void Story_Manager::DrawDuplicateNameWarning() const
{
	const RectF rect(Arg::center = Scene::Center(), 300, 100);
	rect.draw(ColorF(0.1, 0.1, 0.1, 0.9)).drawFrame(2, Palette::Red);

	m_font(U"名前が重複しています。").drawAt(rect.center().movedBy(0, -10), Palette::White);
	m_promptFont(U"Z : 確認").draw(Arg::bottomRight(rect.br().movedBy(-15, -10)), Palette::White);
}

bool Story_Manager::IsActive() const
{
	return m_isActive;
}

StoryID Story_Manager::GetLastStoryID() const
{
	return m_lastStoryID;
}

Point Story_Manager::FindNextValidCursorPos(Point currentPos, int dx, int dy)
{
	Point nextPos = currentPos.movedBy(dx, dy);

	while (nextPos.y >= 0 && nextPos.y < m_gridRows &&
		nextPos.x >= 0 && nextPos.x < m_gridCols)
	{
		if (m_charGrid[nextPos.y][nextPos.x] != U" ")
		{
			return nextPos;
		}
		nextPos.moveBy(dx, dy);
	}
	return currentPos;
}

StoryRenderType Story_Manager::GetRenderType() const
{
	return m_currentRenderType;
}

String Story_Manager::GetCurrentBGM() const
{
	return m_currentBGM;
}
