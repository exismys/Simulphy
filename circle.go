package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type Circle struct {
	pos, vel, acc rl.Vector2
	radius        float32
	col           rl.Color
}

func (c *Circle) draw() {
	rl.DrawCircle(int32(c.pos.X), int32(c.pos.Y), float32(c.radius), c.col)
}

func (c *Circle) move() {
	deltaTime := fixedDeltaTime

	// Calculate velocity
	c.vel.X += c.acc.X * deltaTime
	c.vel.Y += c.acc.Y * deltaTime

	// Calculate position
	c.pos.X += c.vel.X * deltaTime
	c.pos.Y += c.vel.Y * deltaTime

	// Handle Collision
	if c.pos.X-c.radius < 0 {
		c.pos.X = 0 + c.radius
		c.vel.X *= -1 * dampingFactor
	} else if c.pos.X+c.radius > float32(simWidth) {
		c.pos.X = float32(simWidth) - c.radius
		c.vel.X *= -1 * dampingFactor
	}

	if c.pos.Y-c.radius < 0 {
		c.pos.Y = 0 + c.radius
		c.vel.Y *= -1 * dampingFactor
	} else if c.pos.Y+c.radius > float32(simHeight) {
		c.pos.Y = float32(simHeight) - c.radius
		c.vel.Y *= -1 * dampingFactor
	}
}
