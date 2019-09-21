#include "visuals.hpp"
#include "../hooks.hpp"
#include "legitbot.hpp"
#include "ragebot.hpp"

void Render::CreateFonts()
{
	Visuals = g_VGuiSurface->CreateFont_();
	Watermark = g_VGuiSurface->CreateFont_();

	g_VGuiSurface->SetFontGlyphSet(Visuals, "Tahoma", 12, 400, 0, 0, FONTFLAG_OUTLINE);
	g_VGuiSurface->SetFontGlyphSet(Watermark, "Verdana", 16, 600, 0, 0, FONTFLAG_OUTLINE);
}
void Render::Text(int X, int Y, const char* Text, vgui::HFont Font, Color DrawColor, bool Center)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());

	g_VGuiSurface->DrawSetTextFont(Font);
	g_VGuiSurface->DrawSetTextColor(DrawColor);

	if (Center)
	{
		int TextWidth, TextHeight;
		Render::Get().TextSize(TextWidth, TextHeight, Text, Font);
		g_VGuiSurface->DrawSetTextPos(X - TextWidth / 2, Y);
	}
	else
		g_VGuiSurface->DrawSetTextPos(X, Y);
	g_VGuiSurface->DrawPrintText(WText.c_str(), wcslen(WText.c_str()));
}
void Render::TextSize(int& Width, int& Height, const char* Text, vgui::HFont Font)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());
	g_VGuiSurface->GetTextSize(Font, WText.c_str(), Width, Height);
}
void Render::FilledRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawFilledRect(X1, Y1, X2, Y2);
}
void Render::OutlinedRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawOutlinedRect(X1, Y1, X2, Y2);
}
void Render::Line(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawLine(X1, Y1, X2, Y2);
}
Chams::Chams()
{
	std::ofstream("csgo\\materials\\metallic.vmt") << (R"#("VertexLitGeneric" {
	  "$basetexture" "vgui/white"
      "$envmap" "env_cubemap"
      "$normalmapalphaenvmapmask" "1"
      "$envmapcontrast"  "1"
      "$nofog" "1"
      "$model" "1"
      "$nocull" "0"
      "$selfillum" "1"
      "$halflambert" "1"
      "$znearer" "0"
      "$flat" "1" 
}
)#");
}
Chams::~Chams()
{
	std::remove("csgo\\materials\\metallic.vmt");
}
void Chams::OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	// Thanks smef *insert lenny face*
 
	const auto mdl = info.pModel;
	static auto vis = Color(150, 200, 60); //big skeet color
	static auto normal = g_MatSystem->FindMaterial("metallic", TEXTURE_GROUP_MODEL);

	C_BasePlayer* entity = C_BasePlayer::GetPlayerByIndex(info.entity_index);

	if (strstr(mdl->szName, "models/player") != nullptr);
	{
		if (g_LocalPlayer && entity && entity->IsAlive() && !entity->IsDormant())
		{
			g_RenderView->SetColorModulation(vis[0] / 255.f, vis[1] / 255.f, vis[2] / 255.f);
			g_MdlRender->ForcedMaterialOverride(normal);
		}
	}
}

Glow::Glow()
{
}

Glow::~Glow()
{
	// We cannot call shutdown here unfortunately.
	// Reason is not very straightforward but anyways:
	// - This destructor will be called when the dll unloads
	//   but it cannot distinguish between manual unload 
	//   (pressing the Unload button or calling FreeLibrary)
	//   or unload due to game exit.
	//   What that means is that this destructor will be called
	//   when the game exits.
	// - When the game is exiting, other dlls might already 
	//   have been unloaded before us, so it is not safe to 
	//   access intermodular variables or functions.
	//   
	//   Trying to call Shutdown here will crash CSGO when it is
	//   exiting (because we try to access g_GlowObjManager).
	//
}

void Glow::Shutdown()
{
	for (int i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) 
	{
		GlowObjectDefinition_t & glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		C_BasePlayer* entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

		if (glowObject.IsUnused())
			continue;

		if (!entity || entity->IsDormant())
			continue;

		glowObject.m_flAlpha = 0.0f;
	}
}

void Glow::Run()
{
	if (Variables.VisualsGlowEnabled)
	{
		for (int i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) 
		{
			GlowObjectDefinition_t & glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
			C_BasePlayer* entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

			if (glowObject.IsUnused())
				continue;

			if (!entity || entity->IsDormant())
				continue;

			ClassId class_id = entity->GetClientClass()->m_ClassID;
			Color color = Color{};

			switch (class_id) 
			{
			case ClassId_CCSPlayer:

				if (!entity->IsAlive() || !entity->IsEnemy())
					continue;

				color = Color(
					int(255 * 255),
					int(255 * 255),
					int(255 * 255),
					int(255));
				break;
			}

			glowObject.m_flRed = color.r() / 255.0f;
			glowObject.m_flGreen = color.g() / 255.0f;
			glowObject.m_flBlue = color.b() / 255.0f;
			glowObject.m_flAlpha = color.a() / 255.0f;
			glowObject.m_bRenderWhenOccluded = true;
			glowObject.m_bRenderWhenUnoccluded = false;
			glowObject.m_nGlowStyle = Variables.VisualsGlowGlowstyle;
		}
	}
}

RECT Visuals::GetBBox(C_BasePlayer* Player, Vector TransformedPoints[])
{
	RECT rect{};
	ICollideable* collideable = Player->GetCollideable();

	if (!collideable)
		return rect;

	Vector min = collideable->OBBMins();
	Vector max = collideable->OBBMaxs();

	const matrix3x4_t& trans = Player->m_rgflCoordinateFrame();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];

	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector pos = Player->GetAbsOrigin();
	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++)
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
		else
			TransformedPoints[i] = screen_points[i];

	float left = screen_points[0].x;
	float top = screen_points[0].y;
	float right = screen_points[0].x;
	float bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++)
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}
bool Visuals::Begin(C_BasePlayer* Player)
{
	Context.Player = Player;

	if (!Context.Player->IsEnemy())
		return false;

	if (Context.Player->IsDormant())
		return false;

	Vector head = Context.Player->GetHitboxPos(HITBOX_HEAD);
	Vector origin = Context.Player->GetAbsOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, Context.HeadPos) || !Math::WorldToScreen(origin, Context.Origin))
		return false;

	Vector points_transformed[8];
	RECT Box = GetBBox(Context.Player, points_transformed);

	Context.Box = Box;
	Context.Box.top = Box.bottom;
	Context.Box.bottom = Box.top;

	return true;
}

void Visuals::Box()
{
	Render::Get().OutlinedRectange(Context.Box.left - 1, Context.Box.top - 1, Context.Box.right + 1, Context.Box.bottom + 1, Color::Black);
	Render::Get().OutlinedRectange(Context.Box.left + 1, Context.Box.top + 1, Context.Box.right - 1, Context.Box.bottom - 1, Color::Black);
	Render::Get().OutlinedRectange(Context.Box.left, Context.Box.top, Context.Box.right, Context.Box.bottom, Color::White);
}
void Visuals::Name()
{
	player_info_t PlayerInfo;
	g_EngineClient->GetPlayerInfo(Context.Player->EntIndex(), &PlayerInfo);

	int TextWidth, TextHeight;
	Render::Get().TextSize(TextWidth, TextHeight, PlayerInfo.szName, Render::Get().Visuals);
	Render::Get().Text(Context.Box.left + (Context.Box.right - Context.Box.left) / 2, Context.Box.top - TextHeight, PlayerInfo.szName, Render::Get().Visuals, Color(255, 255, 255, 255), true);
}
void Visuals::Weapon()
{
	C_BaseCombatWeapon* Weapon = Context.Player->m_hActiveWeapon();
	if (!Weapon) return;

	std::string WeaponName = std::string(Weapon->GetCSWeaponData()->szHudName + std::string("(") + std::to_string(Weapon->m_iClip1()) + std::string("/") + std::to_string(Weapon->m_iPrimaryReserveAmmoCount()) + std::string(")"));
	WeaponName.erase(0, 13);
	int TextWidth, TextHeight;

	Render::Get().TextSize(TextWidth, TextHeight, WeaponName.c_str(), Render::Get().Visuals);
	Render::Get().Text(Context.Box.left + (Context.Box.right - Context.Box.left) / 2, Context.Box.bottom - 1, WeaponName.c_str(), Render::Get().Visuals, Color(255, 255, 255, 255), true);
}
void Visuals::Health()
{
	int HealthValue = Context.Player->m_iHealth();
	std::clamp(HealthValue, 0, 100);

	float Height = (Context.Box.bottom - Context.Box.top) * float(HealthValue / 100.f);

	Render::Get().FilledRectange(Context.Box.left - 7, Context.Box.top - 1, Context.Box.left - 2, Context.Box.bottom + 1, Color(0, 0, 0, 150));
	Render::Get().FilledRectange(Context.Box.left - 6, Context.Box.top, Context.Box.left - 3, Context.Box.top + Height, Color(0, 220, 50, 255));
}