#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__

#define MAX_COLLIDERS 300

#include "Source/Module.h"
#include "Collider.h"

class Collisions : public Module
{
public:
	// Constructor
	// Fills all collision matrix data
	Collisions();

	// Destructor
	~Collisions();

	// Called at the beginning of the application loop
	// Removes all colliders pending to delete
	// Checks for new collisions and calls its listeners
	bool PreUpdate() override;

	// Called at the middle of the application loop
	// Switches the debug mode on/off
	bool Update(float dt) override;

	// Called at the end of the application loop
	// Draw all colliders (if debug mode is enabled)
	bool PostUpdate() override;

	// Removes all existing colliders
	bool CleanUp() override;

	//void OnCollision(Collider* c1, Collider* c2) override;

	// Adds a new collider to the list
	Collider* AddCollider(SDL_Rect rect, Collider::Type type, Enemy* point_ = nullptr, Module* listener = nullptr);

	// Removes the collider memory and removes it from the colliders array
	void RemoveCollider(Collider* collider);

	void RemoveColliderType(Collider::Type type);

	// Draws all existing colliders with some transparency
	void DebugDraw();

	// All existing colliders in the scene
	Collider* colliders[MAX_COLLIDERS] = { nullptr };

	// The collision matrix. Defines the interaction for two collider types
	// If set two false, collider 1 will ignore collider 2
	bool matrix[Collider::Type::MAX][Collider::Type::MAX];

	// Simple debugging flag to draw all colliders
	bool debug = false;
};

#endif // __MODULE_COLLISIONS_H__

#pragma once
