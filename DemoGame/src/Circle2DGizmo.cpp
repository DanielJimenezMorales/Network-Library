#include "Circle2DGizmo.h"
#include "CameraComponent.h"

void CircleGizmo::RenderConcrete(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const
{
	// Start at the top of the circle
	float x = _radius;
	float y = 0;

	// Decision parameter (initial value)
	float decisionOver2 = 1 - x;

	// We loop until x < y, covering one octant
	while (y <= x) {
		// Draw the points in all 8 octants
		Vec2f renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(x, y));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(-x, y));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(x, -y));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(-x, -y));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(y, x));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(-y, x));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(y, -x));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());
		renderPosition = cameraComponent.ConvertFromWorldPositionToScreenPosition(_position + Vec2f(-y, -x));
		SDL_RenderDrawPoint(renderer, renderPosition.X(), renderPosition.Y());

		// Move to the next pixel vertically
		y++;

		// Update the decision parameter based on the previous pixel's decision
		if (decisionOver2 <= 0) {
			// If the decision parameter is less than or equal to zero, 
			// we move horizontally right (x stays the same, y increases)
			decisionOver2 += 2 * y + 1;
		}
		else {
			// Otherwise, we move diagonally down-right (x decreases, y increases)
			x--;
			decisionOver2 += 2 * (y - x) + 1;
		}
	}
}
