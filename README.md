glance
======

Pure Data objects for Modern OpenGL. This set of objects works fundamentally different from GEM, mainly in the input and vertex declaration departments, so beware.

Realized as a project under the [SuperLab](http://superlab.superuber.com) OpenGL group. HIGHLY EXPERIMENTAL.



objects
---

* **[gl.win &lt;name&gt;]**

  Creates a window, with an optional name. gl.win objects with the same name control the same window, and any unnamed gl.win object controls the default window.

  The outlet emits events targeted to the created window, as lists. The events are: (TODO)

  Messages:

  - [title string with spaces and stuff [
    
    sets a title for this window

  - [create[
    
    creates the window (does not render anything until sent a float)
    
  - [destroy[
    
    destroys the window, if created
    
  - [1[
    
  	starts rendering (i.e. sends render messages to the gl.head objects with the same name)

  - [0[
    
  	stops rendering

* **[gl.head &lt;name&gt;]**
  
  Sets up a rendering head, with an optional name matching a [gl.win] object - no name means the default window.
  Any [gl.*] objects connected to it's outlet will be called to render when the corresponding window is rendering.


* **[gl.enable capability]** / **[gl.disable capability]**
  
  Enables or disables a given OpenGL capability. Capabilities are the OpenGL ones:

  - DEPTH_TEST

  - STENCIL_TEST

  - BLEND

  - LINE_SMOOTH

  - MULTISAMPLE

  - CULL_FACE

  - PROGRAM_POINT_SIZE

  - POLYGON_OFFSET_FILL

  - POLYGON_OFFSET_LINE

  - POLYGON_OFFSET_POINT

  - CLIP_DISTANCE[0...7]

  - PRIMITIVE_RESTART

  - SAMPLE_COVERAGE

  - SAMPLE_MASK


* **[gl.clear bits...]**
  
  Clears OpenGL render buffers. You can specify the clearing of more than one buffer at the same time.

  Buffers are COLOR, DEPTH and STENCIL.

  Messages:

  - [color r g b[

    sets the color value to clear the color buffer with

  - [depth d[
    
    sets the fragment depth to clear the fragment buffer with

  - [stencil v[
    
    sets the value to clear the stencil buffer with

* **[gl.viewport x y width height]**

  Sets up a viewport region in the window


* **[gl.vertexarray usage components length]**

  Sets up a vertex attribute array with an amount of groups of floats. 

  usage is one of the following:

  - STATIC_COPY

  - STATIC_DRAW

  - STATIC_READ

  - DYNAMIC_COPY

  - DYNAMIC_DRAW

  - DYNAMIC_READ

  - STREAM_COPY

  - STREAM_DRAW

  - STREAM_READ

* **[gl.drawarrays]** / **[gl.drawelements]**

  Renders a set of elements in the previously attached vertex arrays, either directly (arrays) or indirectly (elements).

* **[gl.uniform1/2/3/4f]**
  
  Sets up uniform information for shaders

* **[gl.uniformmatrix2/3/4f]**

  Sets up uniform matrices for shaders

* **[gl.texture]**

  Sets up and loads textures

* **[gl.shader]**
  
  Sets up and loads vertex/fragment/geometry shader programs



How to build/run
---

### On OS X:

Download the SDL2 source from hg.libsdl.org

Install FreeImage (if installing from homebrew on Mac OS X 64 bit, do it with --universal to build a 32 bit compatible version)

Build it with Xcode

Install glance.pd_darwin into Pd

Enjoy



TODO
---

Port to Windows/Linux (add GLEW or something, should help with newer than 3.2 stuff on OS X too)

