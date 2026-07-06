# Rössler Attractor


## Made By ASHER



A real-time 3D visualization of the **Rössler attractor**, a classic chaotic dynamical system. The program uses native Windows and OpenGL to render one million iterated points with a rotating camera, phase portrait sidebar, and the governing equations.



---

## How to Run

### Prerequisites

- **Windows** (uses Win32 API and OpenGL)
- **C++ compiler**: Visual Studio (MSVC) or MinGW-w64 with GCC
- **OpenGL**: Provided by Windows (OpenGL32) and your graphics drivers. No extra SDK needed for basic GL; headers may come from your compiler’s SDK or from a bundle like [freeglut](https://www.transmissionzero.co.uk/software/freeglut-devel/) (headers only).

### Build and run (Visual Studio)

1. Create a new **Empty Project** (C++).
2. Add `main.cpp` to the project.
3. Link OpenGL:
   - **Project → Properties → Linker → Input → Additional Dependencies**  
     Add: `opengl32.lib`
4. Build (e.g. **Build → Build Solution**).
5. Run the executable. A window titled **“Rossler Attractor”** opens with the 3D view and sidebar.

### Build and run (command line with MinGW)

```bash
g++ -o rossler.exe main.cpp -lopengl32 -lgdi32
rossler.exe
```

### Build and run (command line with MSVC)

```cmd
cl main.cpp /Fe:rossler.exe opengl32.lib gdi32.lib user32.lib
rossler.exe
```

Close the window or press Alt+F4 to exit.

---

## How This Attractor Was Made

### Modules and technologies

| Component        | Role |
|-----------------|------|
| **Windows API** (`windows.h`) | Window creation, message loop, device context (DC), pixel format. |
| **OpenGL** (`GL/gl.h`)       | 3D projection, points, 2D ortho for sidebar, text via `wglUseFontBitmaps`. |
| **C++ standard library**     | `std::vector`, `std::sqrt`, `rand`, `time`, etc. |
| **No external math/plot libs** | All dynamics and rendering are implemented directly in `main.cpp`. |

So: one source file, Windows + OpenGL + standard C++. No extra “attractor” or math libraries; the Rössler system and Runge–Kutta integrator are hand-coded.

### Pipeline (high level)

1. **Integration**  
   The Rössler ODEs are integrated with **4th-order Runge–Kutta (RK4)** and a fixed time step `dt = 0.005`. Initial point `(0.1, 0.1, 0.1)`; 5000 steps of “spin-up” then 1,000,000 steps are stored.

2. **Preprocessing**  
   Stored points are recentered (subtract mean), scaled by `SCALE = 0.18`, and a small random offset is added per point for a slight spread effect.

3. **Rendering**  
   - **Main view**: 3D perspective, rotating camera; a sliding window of 400,000 points is drawn as GL points with color and alpha based on index and time.  
   - **Sidebar**: 2D orthographic “phase portrait” (x–y from the same trajectory) and a box showing the equations and parameters.

---

## Maths Used

### Rössler system (ODEs)

The attractor is defined by three coupled ordinary differential equations:

\[
\begin{aligned}
\frac{dx}{dt} &= -y - z \\
\frac{dy}{dt} &= x + a\,y \\
\frac{dz}{dt} &= b + z\,(x - c)
\end{aligned}
\]

In the code, the default parameters are:

- \( a = 0.2 \)
- \( b = 0.2 \)
- \( c = 5.7 \)

These are the “standard” Rössler parameters that produce a single chaotic attractor with one lobe.

### Numerical integration: 4th-order Runge–Kutta (RK4)

The system is advanced in time using RK4. For a vector ODE \( \mathbf{u}' = \mathbf{f}(\mathbf{u}) \), one step of size \( \Delta t \) is:

\[
\begin{aligned}
\mathbf{k}_1 &= \mathbf{f}(\mathbf{u}_n), \\
\mathbf{k}_2 &= \mathbf{f}(\mathbf{u}_n + \tfrac{\Delta t}{2}\mathbf{k}_1), \\
\mathbf{k}_3 &= \mathbf{f}(\mathbf{u}_n + \tfrac{\Delta t}{2}\mathbf{k}_2), \\
\mathbf{k}_4 &= \mathbf{f}(\mathbf{u}_n + \Delta t\,\mathbf{k}_3), \\
\mathbf{u}_{n+1} &= \mathbf{u}_n + \frac{\Delta t}{6}(\mathbf{k}_1 + 2\mathbf{k}_2 + 2\mathbf{k}_3 + \mathbf{k}_4).
\end{aligned}
\]

In the code, \( \mathbf{u} = (x,y,z) \) and \( \mathbf{f} \) is the right-hand side of the Rössler system above; this is implemented in `rosslerStep()` with `dt = 0.005`.

### Chaos and the attractor

- The system is **deterministic** but **chaotic**: tiny changes in initial conditions lead to very different long-term trajectories.
- The **Rössler attractor** is the set in phase space \( (x,y,z) \) that typical trajectories converge to; it has a characteristic “folded band” or “single scroll” shape.
- The sidebar’s **phase portrait** is the projection of that trajectory onto the \( (x,y) \) plane.

No extra modules or libraries were used for this math; it’s all in `main.cpp` (Rössler RHS + RK4 + scaling/centering).

---

## Download Links & References

### Rössler attractor and chaos

- **Wikipedia – Rössler attractor**  
  [https://en.wikipedia.org/wiki/R%C3%B6ssler_attractor](https://en.wikipedia.org/wiki/R%C3%B6ssler_attractor)  
  Equations, history, and typical parameter values.

- **Wikipedia – Chaos theory**  
  [https://en.wikipedia.org/wiki/Chaos_theory](https://en.wikipedia.org/wiki/Chaos_theory)  
  General background on deterministic chaos.

### Numerical methods (RK4)

- **Wikipedia – Runge–Kutta methods**  
  [https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods](https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods)  
  Derivation and formulas for RK4.

- **Numerical Recipes (optional book)**  
  [http://numerical.recipes/](http://numerical.recipes/)  
  Standard reference for ODE integration; RK4 is in the ODE chapter.

### OpenGL and Windows

- **OpenGL – OpenGL Wiki**  
  [https://www.khronos.org/opengl/wiki/](https://www.khronos.org/opengl/wiki/)  
  Concepts and API overview.

- **Microsoft – OpenGL (Windows)**  
  [https://docs.microsoft.com/en-us/windows/win32/opengl/opengl](https://docs.microsoft.com/en-us/windows/win32/opengl/opengl)  
  Using OpenGL on Windows (opengl32.lib, WGL).

- **freeglut (optional, for GL headers/context)**  
  [https://www.transmissionzero.co.uk/software/freeglut-devel/](https://www.transmissionzero.co.uk/software/freeglut-devel/)  
  Can be used to get GL headers; this project only needs `opengl32.lib` and a way to get `GL/gl.h` (e.g. from your compiler’s SDK).

### Original paper (academic)

- **O. E. Rössler, “An equation for continuous chaos,” *Physics Letters A*, 1976.**  
  [https://doi.org/10.1016/0375-9601(76)90101-8](https://doi.org/10.1016/0375-9601(76)90101-8)  
  Original definition of the Rössler system.

---

## Summary

| Item | Description |
|------|-------------|
| **What it is** | Real-time 3D + 2D visualization of the Rössler chaotic attractor. |
| **Math** | Rössler ODEs integrated with RK4; no external math libraries. |
| **Modules** | Windows API, OpenGL, C++ standard library; single file `main.cpp`. |
| **Run** | Build with MSVC or MinGW, link `opengl32.lib` (and `gdi32`/`user32` as needed), then run the executable. |
| **References** | Links above cover the maths (Rössler, RK4), chaos theory, and OpenGL/Windows. |
