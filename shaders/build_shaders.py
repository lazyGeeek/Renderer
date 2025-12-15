import os;

script_dir = os.path.dirname(os.path.abspath(__file__))

os.system("glslc " + os.path.join(script_dir, "triangle_vert.vert") + " -o " + os.path.join(script_dir, "triangle_vert.spv"))
os.system("glslc " + os.path.join(script_dir, "triangle_frag.frag") + " -o " + os.path.join(script_dir, "triangle_frag.spv"))
