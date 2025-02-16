#include "pch.h"
#include "VenoXRLAI.h"
#define _USE_MATH_DEFINES
#include <cmath>

BAKKESMOD_PLUGIN(VenoXRLAI, "Better than your mom", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void VenoXRLAI::onLoad()
{
	_globalCvarManager = cvarManager;
	LOG("VenoX Plugin loaded! - with ball on top");

	cvarManager->registerNotifier("ballontop", [this](std::vector<std::string> args) {
		ballOnTop();
	}, "", PERMISSION_ALL);

	gameWrapper->HookEventWithCaller<CarWrapper>(
		"Function TAGame.Car_TA.SetVehicleInput",
		std::bind(&VenoXRLAI::on_tick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	);

	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("VenoXRLAI debug mode enabled");

	// LOG and DEBUGLOG use fmt format strings https://fmt.dev/latest/index.html
	//DEBUGLOG("1 = {}, 2 = {}, pi = {}, false != {}", "one", 2, 3.14, true);

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	LOG("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	LOG("the cvar with name: {} changed", cvarName);
	//	LOG("the new value is: {}", newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&VenoXRLAI::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&VenoXRLAI::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	LOG("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&VenoXRLAI::YourPluginMethod, this);
}
void VenoXRLAI::ballOnTop() {
	/*
	if (!gameWrapper->IsInFreeplay()) {
		LOG("Player is not in Freeplay");
		return; }
	*/
	ServerWrapper server = gameWrapper->GetCurrentGameState();

	if (!server) { 
		LOG("Server is none");
		return;
	}

	BallWrapper ball = server.GetBall();
	if (!ball) { 
		LOG("Ball is none");
		return; }
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) { 
		LOG("Car is none");
		return; }

	Vector carVelocity = car.GetVelocity();
	ball.SetVelocity(carVelocity);

	Vector carLocation = car.GetLocation();
	float ballRadius = ball.GetRadius();
	ball.SetLocation(carLocation + Vector{ 0, 0, ballRadius * 2 });
	LOG("Executed SetLocation");

	//car.ForceBoost(true);
}

void VenoXRLAI::Attack() {
	ServerWrapper server = gameWrapper->GetCurrentGameState();

	if (!server) {
		LOG("Server is none");
		return;
	}

	BallWrapper ball = server.GetBall();
	if (!ball) {
		LOG("Ball is none");
		return;
	}
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) {
		LOG("Car is none");
		return;
	}


}

Vector VenoXRLAI::RotatorToVector(Rotator rot)
{
	float yaw = rot.Yaw * (M_PI / 32768.0f);
	float pitch = rot.Pitch * (M_PI / 32768.0f);

	return Vector(cos(yaw) * cos(pitch), sin(yaw) * cos(pitch), sin(pitch));
}

Vector Normalize(Vector vec)
{
	float length = sqrt(vec.X * vec.X + vec.Y * vec.Y + vec.Z * vec.Z);
	if (length == 0) return Vector(0, 0, 0); // Vermeidung von Division durch 0
	return Vector(vec.X / length, vec.Y / length, vec.Z / length);
}

float Dot(Vector a, Vector b)
{
	return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}

void VenoXRLAI::on_tick(CarWrapper car, void* params, std::string eventName) {
	if (!car) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return;

	BallWrapper ball = server.GetBall();
	if (!ball) return;

	ControllerInput* input = reinterpret_cast<ControllerInput*>(params);
	if (!input) return;

	// Ball- und Autoposition abrufen
	Vector carPos = car.GetLocation();
	Vector ballPos = ball.GetLocation();

	// Richtung zum Ball berechnen
	Vector toBall = Normalize(ballPos - carPos);

	// Autovorwärtsrichtung abrufen
	Rotator carRot = car.GetRotation();
	Vector forward = RotatorToVector(carRot);

	// Winkel zwischen Auto-Vorwärtsrichtung und Ballrichtung berechnen
	float angle = Dot(forward, toBall);

	// 🚀 Standardmäßig Gas geben
	input->Throttle = 1.0f;

	// 🤖 Falsch ausgerichtet? Dann lenken und ggf. rückwärts fahren
	if (angle < -0.1f) {
		// Falls das Auto stark in die falsche Richtung zeigt, rückwärts fahren
		input->Throttle = -1.0f;
		input->Steer = (toBall.Y > forward.Y) ? -1.0f : 1.0f;
	}
	else if (angle < 0.5f) {
		// Falls das Auto leicht falsch ausgerichtet ist, lenken
		input->Steer = (toBall.Y > forward.Y) ? 1.0f : -1.0f;
	}

	// 🚀 Falls wir halbwegs richtig ausgerichtet sind, Boosten
	if (angle > 0.8f) {
		input->HoldingBoost;
	}

	// 🏀 Falls der Ball in der Luft ist, evtl. springen
	if (ballPos.Z > 500.0f) {
		input->Jump = 1;
	}
}

