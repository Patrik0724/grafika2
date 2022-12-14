
//=============================================================================================
// Mintaprogram: Zold haromszog. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Benyovszki Patrik
// Neptun : IQ00CM
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const float epsilon = 0.001;

mat4 transponate(mat4 mat) {
	return mat4(mat[0][0], mat[1][0], mat[2][0], mat[3][0],
		mat[0][1], mat[1][1], mat[2][1], mat[3][1],
		mat[0][2], mat[1][2], mat[2][2], mat[3][2],
		mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
}

struct Material {
	vec3 ka, kd, ks;
	float shininess;
	Material(vec3 _kd, vec3 _ks, float _shininess) : ka(_kd* M_PI), kd(_kd), ks(_ks)
	{
		shininess = _shininess;
	}
};

struct Hit {
	float t;
	vec3 position, normal;
	Material* material;
	Hit() { t = -1; }
};

struct Ray {
	vec3 start, dir;
	Ray(vec3 _start, vec3 _dir) : start(_start) { dir = normalize(_dir); }
};

class Intersectable {
protected:
	Material* material;
public:
	virtual Hit intersect(const Ray& ray) = 0;
};

struct Quadratic : Intersectable {
	mat4 Q, Qmax;
	vec3 translation;
	Material* material;
	float phi;
	vec3 w;
	vec4 basePoint = vec4(0, 0, 0, 1);

	Quadratic(mat4 _Q, mat4 _Qmax, vec3 _translation, Material * mat, float _phi, vec3 _w)
		: Q(_Q), Qmax(_Qmax), translation(_translation), w(_w)
	{
		phi = _phi;
		material = mat;

		mat4 rotation = RotationMatrix(-phi, w);
		mat4 rotationT = transponate(rotation);

		Q = TranslateMatrix(-translation) * rotation * Q * rotationT * transponate(TranslateMatrix(-translation));
		Qmax = TranslateMatrix(-translation) * rotation * Qmax * rotationT * transponate(TranslateMatrix(-translation));

		basePoint = basePoint * TranslateMatrix(translation) * RotationMatrix(phi, w);
	}

	vec3 gradf(vec3 r) {
		vec4 v = Q[0] * r.x + Q[1] * r.y + Q[2] * r.z + Q[3];
		vec4 g = vec4(r.x, r.y, r.z, 1) * Q + v;
		return vec3(g.x, g.y, g.z);
	}

	Hit intersect(const Ray& ray) {
		Hit hit;
		vec3 start = ray.start;
		vec4 S(start.x, start.y, start.z, 1), D(ray.dir.x, ray.dir.y, ray.dir.z, 0);
		float a = dot(D * Q, D), b = dot(D * Q, S) + dot(S * Q, D), c = dot(S * Q, S);
		float  discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);

		float t1 = (-b + sqrt_discr) / 2.0f / a;
		vec3 p1 = start + ray.dir * t1;
		

		float t2 = (-b - sqrt_discr) / 2.0f / a;
		vec3 p2 = start + ray.dir * t2;
		
		
		vec4 p = vec4(p1.x, p1.y, p1.z, 1);
		if (dot(p * Qmax, p) < 0)
			t1 = -1;
		vec4 q = vec4(p2.x, p2.y, p2.z, 1);
		if (dot(q * Qmax, q) < 0)
			t2 = -1;
		

		if (t1 <= 0 && t2 <= 0) return hit;
		if (t1 <= 0) hit.t = t2;
		else if (t2 <= 0) hit.t = t1;
		else if (t2 < t1) hit.t = t2;
		else hit.t = t1;
		hit.position = start + ray.dir * hit.t;
		hit.position = hit.position;
		hit.normal = normalize(gradf(hit.position));
		hit.material = material;
		
		
		return hit;
	}

	void animate(vec3 w, float phi, vec3 trans) {
		Q = TranslateMatrix(vec3(-trans)) * RotationMatrix(-phi, w) * TranslateMatrix(vec3(trans)) * Q * transponate(TranslateMatrix(vec3(trans))) * transponate(RotationMatrix(-phi, w)) * transponate(TranslateMatrix(vec3(-trans)));
		Qmax = TranslateMatrix(vec3(-trans)) * RotationMatrix(-phi, w) * TranslateMatrix(vec3(trans)) * Qmax * transponate(TranslateMatrix(vec3(trans))) * transponate(RotationMatrix(-phi, w)) * transponate(TranslateMatrix(vec3(-trans)));
		basePoint = basePoint * TranslateMatrix(vec3(-trans)) * RotationMatrix(phi, w) * TranslateMatrix(vec3(trans));
	}
};

class Camera {
	vec3 eye, lookat, right, up;
	float fov;
public:
	void set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov) {
		eye = _eye;
		lookat = _lookat;
		fov = _fov;
		vec3 w = eye - lookat;
		float focus = length(w) * tanf(fov / 2);
		right = normalize(cross(vup, w)) * focus;
		up = normalize(cross(w, right)) * focus;
	}
	Ray getRay(int X, int Y) {
		vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
		return Ray(eye, dir);
	}

	void animate(float dt) {
		vec3 d = eye - lookat;
		eye = vec3(d.x * cos(dt) + d.y * sin(dt) + lookat.x, d.y * cos(dt) - d.x * sin(dt) + lookat.y, eye.z);
		set(eye, lookat, vec3(0, 0, 1), fov);
	}
};

struct Light {
	vec3 pos, Le;
	Light(vec3 _pos, vec3 _Le) {
		pos = _pos;
		Le = _Le;
	}

	void addTransformation(mat4 transformation) {
		vec4 pos4 = vec4(pos.x, pos.y, pos.z, 1);
		vec4 newpos4 = pos4 * transformation;
		pos = vec3(newpos4.x, newpos4.y, newpos4.z);
	}

	void animate(vec3 w, float phi, vec3 translation) {
		vec4 pos4 = vec4(pos.x, pos.y, pos.z, 1);
		vec4 newpos4 = pos4 * TranslateMatrix(-translation) * RotationMatrix(phi, w) * TranslateMatrix(translation);
		pos = vec3(newpos4.x, newpos4.y, newpos4.z);
	}
};

struct LampElement {
	Quadratic* surface;
	LampElement* next;
	LampElement(Quadratic* _surface, LampElement* _next) {
		surface = _surface;
		next = _next;
	}

	void animate(vec3 w, float phi, vec4 translation) {
		for (LampElement* element = this; element != NULL; element = element->next)
			element->surface->animate(w, phi, vec3(translation.x, translation.y, translation.z));
	}
};

struct Lamp {
	LampElement* start;
	Lamp(LampElement* _start) {
		start = _start;
	}

	vec3 animate(vec3 w, float phi, int i) {
		LampElement* first = start;
		for (int j = 0; j < i; ++j)
			first = first->next;
		vec4 translation = first->surface->basePoint;
		first->animate(w, phi, first->surface->basePoint);
		return vec3(translation.x, translation.y, translation.z);
	}
};

class Scene {
	Lamp* l;
	std::vector<Light*> lights;
	Camera cam;
	vec3 La;
public:
	Scene() {
		vec3 eye = vec3(2, 2, 4), vup = vec3(0, 0, 1), lookat = vec3(0, 0, 2);
		float fov = 45 * M_PI / 180;
		cam.set(eye, lookat, vup, fov);

		La = vec3(0.2f, 0.2f, 0.2f);
		vec3 lightPos = vec3(0, 3, 5), Le = vec3(10, 10, 10);
		lights.push_back(new Light(lightPos, Le));
		vec3 ks(2, 2, 2);

		mat4 paraboloid = mat4(-20, 0, 0, 0,
			0, -20, 0, 0,
			0, 0, 0, 4,
			0, 0, 4, 0);

		lights.push_back(new Light(vec3(0, 0, 0.1), vec3(1, 1, 1)));
		mat4 transformation = RotationMatrix(M_PI, vec3(0, 1, 0)) * TranslateMatrix(vec3(0.28, 0.52, 2.13));
		lights.at(1)->addTransformation(transformation);

		LampElement* lampshade = new LampElement(new Quadratic(paraboloid, mat4(ScaleMatrix(vec3(0, 0, -50))), vec3(0.28, 0.52, 2.13), new Material(vec3(0.2, 0.2, 0.5), ks, 50), M_PI, vec3(0, 1, 0)), NULL);	

		LampElement* sphere3 = new LampElement(new Quadratic(ScaleMatrix(vec3(-1000, -1000, -1000)), mat4(ScaleMatrix(vec3(0, 0, 0))), vec3(0.275, 0.5, 2.16), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0, vec3(0, 0, 1)), lampshade);

		LampElement* stick2 = new LampElement(new Quadratic(ScaleMatrix(vec3(-5000, -5000, 0)), mat4(ScaleMatrix(vec3(0, 0, -16))), vec3(0.27, 0.25, 2.05), new Material(vec3(0.2, 0.2, 0.5), ks, 50), -20, vec3(1, 0, 0)), sphere3);

		LampElement* sphere2 = new LampElement(new Quadratic(ScaleMatrix(vec3(-1000, -1000, -1000)), mat4(ScaleMatrix(vec3(0, 0, 0))), vec3(0.27, 0, 1.94), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0, vec3(0, 0, 1)), stick2);

		LampElement* stick1 = new LampElement(new Quadratic(ScaleMatrix(vec3(-5000, -5000, 0)), mat4(ScaleMatrix(vec3(0, 0, -4.5))), vec3(0.12, 0, 1.47), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0.3, vec3(0, 1, 0)), sphere2);

		LampElement* sphere1 = new LampElement(new Quadratic(ScaleMatrix(vec3(-1000, -1000, -1000)), mat4(ScaleMatrix(vec3(0, 0, 0))), vec3(0, 0, 1.08), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0, vec3(0, 0, 1)), stick1);

		LampElement* foottop = new LampElement(new Quadratic(ScaleMatrix(vec3(0, 0, -1)), mat4(ScaleMatrix(vec3(-8, -16, 0))), vec3(0, 0, 0.06), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0, vec3(0, 0, 1)), sphere1);
		
		LampElement* foot = new LampElement(new Quadratic(ScaleMatrix(vec3(-8, -16, 0)), mat4(ScaleMatrix(vec3(0, 0, -1000))), vec3(0, 0, 1.03), new Material(vec3(0.2, 0.2, 0.5), ks, 50), 0, vec3(0, 0, 1)), foottop);

		LampElement* floor = new LampElement (new Quadratic(ScaleMatrix(vec3(0, 0, -1)), mat4(ScaleMatrix(vec3(0, 0, 0))), vec3(0, 0, 0), new Material(vec3(0.5, 0.4, 0.3), ks, 50), 0, vec3(0, 0, 1)), foot);
		

		l = new Lamp(floor);
	};

	void render(std::vector<vec4>& image) {
		for (int Y = 0; Y < windowHeight; ++Y) {
#pragma omp parallel for
			for (int X = 0; X < windowWidth; ++X) {
				vec3 color = trace(cam.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}
	}

Hit firstIntersect(Ray ray) {
	Hit bestHit;
	for (LampElement* element = l->start; element != NULL; element = element->next) {
		Hit hit = element->surface->intersect(ray);
		if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))
			bestHit = hit;
	}
	if (dot(ray.dir, bestHit.normal) > 0)
		bestHit.normal = bestHit.normal * -1;
	return bestHit;
}

vec3 trace(Ray ray) {
	Hit hit = firstIntersect(ray);
	if (hit.t < 0)
		return La;
	vec3 outRadiance = hit.material->ka * La;
	for (Light* light : lights) {
		Ray shadowRay(hit.position + hit.normal * epsilon, normalize(light->pos - hit.position));
		float cosA = dot(hit.normal, normalize(light->pos - hit.position));
		Hit shadowHit = firstIntersect(shadowRay);
		if (cosA > 0 && (shadowHit.t < 0 || shadowHit.t > length(light->pos - hit.position))) {
			vec3 LeIn = light->Le / dot(light->pos - hit.position, light->pos - hit.position);
			outRadiance = outRadiance + LeIn * hit.material->kd * cosA;
			vec3 halfway = normalize(-ray.dir + light->pos - hit.position);
			float cosDelta = dot(hit.normal, halfway);
			if (cosDelta > 0)
				outRadiance = outRadiance + LeIn * hit.material->ks * powf(cosDelta, hit.material->shininess);
		}
	}
	return outRadiance;
}

void animate(float dt) {
	vec3 translation;
	cam.animate(dt / 2);

	translation = l->animate(vec3(0, 0, 1), dt, 3);
	lights.at(1)->animate(vec3(0, 0, 1), dt, translation);
	
	translation = l->animate(vec3(0, 1, 0), dt, 5);
	lights.at(1)->animate(vec3(0, 1, 0), dt, translation);
}
};

GPUProgram gpuProgram;
Scene scene;

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	layout(location = 0) in vec2 cVertexPosition;	
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1)) / 2;
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1);
	}
)";

// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform sampler2D textureUnit;
	in vec2 texcoord;
	out vec4 fragmentColor;

	void main() {
		fragmentColor = texture(textureUnit, texcoord);
	}
)";

class FullScreenTexturedQuad {
	unsigned int vao, textureId;
public:
	FullScreenTexturedQuad(int windowWidth, int windowHeight)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		unsigned int vbo;
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		float vertexCoords[] = { -1, -1, 1, -1, 1, 1, -1, 1 };
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void LoadTexture(std::vector <vec4>& image){
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, &image[0]);
	}

	void Draw() {
		glBindVertexArray(vao);
		int location = glGetUniformLocation(gpuProgram.getId(), "textureUnit");
		const unsigned int textureUnit = 0;
		if (location >= 0) {
			glUniform1i(location, textureUnit);
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, textureId);
		}
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
};

FullScreenTexturedQuad* fullScreenTexturedQuad;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight);

	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

// Window has become invalid: Redraw
void onDisplay() {
	std::vector<vec4> image(windowWidth * windowHeight);
	scene.render(image);

	fullScreenTexturedQuad->LoadTexture(image);
	fullScreenTexturedQuad->Draw();
	glutSwapBuffers();
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;

	char* buttonStat;
	switch (state) {
	case GLUT_DOWN: buttonStat = "pressed"; break;
	case GLUT_UP:   buttonStat = "released"; break;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	scene.animate(0.1f);
	glutPostRedisplay();
}
