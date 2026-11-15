"""
make_office_props.py: TAKE 4471 office props, built procedurally in Blender (5.2).

Run inside Blender: Scripting tab > Open > this file > Run Script
   or headless:      blender -b -P make_office_props.py

Builds 65 low-poly props (office, supplies, plants, lobby, kitchen, mannequins), each with its pivot at the floor (bottom centre),
real-world size in metres, bevelled edges, and named material slots.
Saves SourceArt/Blender/OfficeProps.blend and one FBX per prop in
SourceArt/Blender/Export/  (import into Unreal at /Game/Props/Meshes).

Material slots are placeholders (M_Wood, M_MetalDark, ...). In Unreal swap
them for the Fear-ready grey-box instances or real PBR materials.
"""
import math
import os

import bpy  # noqa: I001  (bpy first: it puts bmesh/mathutils on the path when run as a module)
import bmesh
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__)) if "__file__" in dir() else bpy.path.abspath("//")
EXPORT = os.path.join(HERE, "Export")

PALETTE = {  # slot name -> viewport colour (RGBA)
    "M_Wood": (0.55, 0.38, 0.22, 1), "M_DeskTop": (0.82, 0.80, 0.76, 1),
    "M_MetalDark": (0.12, 0.13, 0.15, 1), "M_MetalLight": (0.62, 0.64, 0.67, 1),
    "M_PlasticWhite": (0.90, 0.91, 0.92, 1), "M_PlasticBlack": (0.05, 0.05, 0.06, 1),
    "M_Fabric": (0.30, 0.36, 0.42, 1), "M_FabricMint": (0.50, 0.75, 0.70, 1),
    "M_Screen": (0.02, 0.03, 0.05, 1), "M_Glass": (0.55, 0.75, 0.95, 1),
    "M_RedPaint": (0.70, 0.07, 0.06, 1), "M_Canvas": (0.12, 0.12, 0.12, 1),
    "M_Ceramic": (0.93, 0.93, 0.90, 1), "M_Paper": (0.97, 0.97, 0.95, 1),
    "M_Yellow": (0.95, 0.78, 0.10, 1), "M_Mattress": (0.80, 0.86, 0.88, 1),
    "M_Leaf": (0.20, 0.55, 0.30, 1), "M_LeafLight": (0.40, 0.70, 0.35, 1), "M_Soil": (0.18, 0.12, 0.08, 1),
    "M_PotWhite": (0.95, 0.95, 0.93, 1), "M_BinderBlue": (0.18, 0.35, 0.70, 1), "M_BinderMint": (0.45, 0.78, 0.70, 1),
    "M_BinderRed": (0.75, 0.20, 0.22, 1), "M_Skin": (0.86, 0.84, 0.80, 1), "M_StalkerSkin": (0.30, 0.31, 0.33, 1),
    "M_Counter": (0.92, 0.92, 0.90, 1), "M_Steel": (0.75, 0.77, 0.79, 1), "M_SofaMint": (0.55, 0.80, 0.74, 1),
    "M_ExitGreen": (0.10, 0.75, 0.35, 1), "M_ClockFace": (0.98, 0.98, 0.96, 1), "M_SkyBlue": (0.65, 0.78, 0.95, 1),
    "M_Cardboard": (0.55, 0.40, 0.25, 1),
    "M_Bread": (0.86, 0.68, 0.42, 1), "M_Crust": (0.62, 0.38, 0.18, 1), "M_Cheese": (0.98, 0.80, 0.30, 1),
    "M_Tomato": (0.80, 0.16, 0.10, 1), "M_Frosting": (0.96, 0.58, 0.72, 1),
    "M_SignWet": (0.95, 0.78, 0.10, 1), "M_SignWetBack": (0.95, 0.78, 0.10, 1), "M_ExitFace": (0.10, 0.75, 0.35, 1),
    "M_SignFace": (0.92, 0.94, 0.95, 1), "M_Mirror": (0.70, 0.78, 0.82, 1), "M_Face": (0.86, 0.84, 0.80, 1),
}


# --------------------------------------------------------------------------
# helpers
# --------------------------------------------------------------------------
def material(name):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.diffuse_color = PALETTE[name]
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = PALETTE[name]
    return mat


class Prop:
    """Collects parts in one bmesh; each part gets a material slot."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.slots = []

    def _slot(self, mat):
        if mat not in self.slots:
            self.slots.append(mat)
        return self.slots.index(mat)

    def _add(self, geom_verts, mat, matrix):
        idx = self._slot(mat)
        faces = {f for v in geom_verts for f in v.link_faces}
        bmesh.ops.transform(self.bm, matrix=matrix, verts=list(geom_verts))
        for f in faces:
            f.material_index = idx

    def box(self, size, loc, mat, rot_z=0.0, rot_x=0.0, rot_y=0.0):
        """Box of size (x, y, z) metres, centred at loc."""
        r = bmesh.ops.create_cube(self.bm, size=1.0)
        m = (Matrix.Translation(Vector(loc)) @ Matrix.Rotation(rot_z, 4, "Z")
             @ Matrix.Rotation(rot_y, 4, "Y") @ Matrix.Rotation(rot_x, 4, "X") @ Matrix.Diagonal((*size, 1)))
        self._add(r["verts"], mat, m)

    def cyl(self, radius, depth, loc, mat, segs=16, radius2=None, axis="Z"):
        r = bmesh.ops.create_cone(self.bm, cap_ends=True, segments=segs,
                                  radius1=radius, radius2=radius if radius2 is None else radius2, depth=depth)
        rot = {"Z": Matrix(), "X": Matrix.Rotation(math.pi / 2, 4, "Y"), "Y": Matrix.Rotation(math.pi / 2, 4, "X")}[axis]
        self._add(r["verts"], mat, Matrix.Translation(Vector(loc)) @ rot)

    def sphere(self, radius, loc, mat, segs=12):
        r = bmesh.ops.create_uvsphere(self.bm, u_segments=segs, v_segments=segs // 2, radius=radius)
        self._add(r["verts"], mat, Matrix.Translation(Vector(loc)))

    def torus(self, major, minor, loc, mat, rot_x=math.pi / 2, segs=16):
        """Simple torus (mug handle) built from a ring of small cylinders."""
        for i in range(segs):
            a0, a1 = 2 * math.pi * i / segs, 2 * math.pi * (i + 1) / segs
            p0 = Vector((math.cos(a0) * major, 0, math.sin(a0) * major))
            p1 = Vector((math.cos(a1) * major, 0, math.sin(a1) * major))
            mid, d = (p0 + p1) / 2, (p1 - p0)
            r = bmesh.ops.create_cone(self.bm, cap_ends=True, segments=8, radius1=minor, radius2=minor, depth=d.length * 1.15)
            rot = Vector((0, 0, 1)).rotation_difference(d.normalized()).to_matrix().to_4x4()
            self._add(r["verts"], mat, Matrix.Translation(Vector(loc) + mid) @ rot)

    def finish(self, bevel=0.004, origin=(0.0, 0.0, 0.0), fit_uv=None):
        """fit_uv: {material: (x0, x1, z0, z1)} gives that material's front faces 0..1 UVs (screens, labels)."""
        mesh = bpy.data.meshes.new(self.name)
        bmesh.ops.remove_doubles(self.bm, verts=self.bm.verts, dist=0.0001)
        if any(origin):
            bmesh.ops.translate(self.bm, vec=Vector(origin) * -1, verts=self.bm.verts)
        # Box-projected UVs (1 UV unit = 1 m) so textures and tangents work in Unreal
        uv = self.bm.loops.layers.uv.new("UVMap")
        for f in self.bm.faces:
            n = f.normal
            ax = max(range(3), key=lambda i: abs(n[i]))
            for loop in f.loops:
                co = loop.vert.co
                loop[uv].uv = (co.y, co.z) if ax == 0 else ((co.x, co.z) if ax == 1 else (co.x, co.y))
            if fit_uv and ax == 2 and f.material_index < len(self.slots) and ("top:" + self.slots[f.material_index]) in fit_uv:
                x0, x1, y0, y1 = fit_uv["top:" + self.slots[f.material_index]]
                for loop in f.loops:
                    co = loop.vert.co
                    loop[uv].uv = ((co.x - x0) / (x1 - x0), (co.y - y0) / (y1 - y0))
            if fit_uv and ax == 1 and f.material_index < len(self.slots) and self.slots[f.material_index] in fit_uv:
                x0, x1, z0, z1 = fit_uv[self.slots[f.material_index]]
                for loop in f.loops:
                    co = loop.vert.co
                    loop[uv].uv = ((co.x - x0) / (x1 - x0), (co.z - z0) / (z1 - z0))
        self.bm.to_mesh(mesh)
        self.bm.free()
        obj = bpy.data.objects.new(self.name, mesh)
        bpy.context.scene.collection.objects.link(obj)
        for m in self.slots:
            obj.data.materials.append(material(m))
        if bevel:
            mod = obj.modifiers.new("Bevel", "BEVEL")
            mod.width, mod.segments, mod.limit_method = bevel, 2, "ANGLE"
            mod.harden_normals = True
        for poly in mesh.polygons:
            poly.use_smooth = True
        return obj


# --------------------------------------------------------------------------
# props (all dimensions in metres; pivot = floor centre)
# --------------------------------------------------------------------------
def desk():
    p = Prop("SM_Desk")
    p.box((1.6, 0.8, 0.035), (0, 0, 0.7425), "M_DeskTop")
    for x in (-0.78, 0.78):
        p.box((0.035, 0.74, 0.725), (x, 0, 0.3625), "M_MetalLight")
    p.box((1.52, 0.02, 0.4), (0, 0.33, 0.45), "M_MetalLight")                 # modesty panel
    p.box((0.42, 0.6, 0.6), (0.5, -0.05, 0.32), "M_MetalLight")               # drawer pedestal
    for i, z in enumerate((0.52, 0.32)):
        p.box((0.40, 0.01, 0.17 if i == 0 else 0.2), (0.5, -0.355, z), "M_PlasticWhite")
        p.box((0.12, 0.015, 0.012), (0.5, -0.365, z + 0.05), "M_MetalDark")
    return p.finish()


def office_chair():
    p = Prop("SM_OfficeChair")
    for i in range(5):
        a = 2 * math.pi * i / 5
        p.box((0.32, 0.045, 0.035), (math.cos(a) * 0.16, math.sin(a) * 0.16, 0.07), "M_MetalDark", rot_z=a)
        p.sphere(0.03, (math.cos(a) * 0.31, math.sin(a) * 0.31, 0.03), "M_PlasticBlack", segs=10)
    p.cyl(0.025, 0.32, (0, 0, 0.24), "M_MetalLight")
    p.box((0.5, 0.48, 0.08), (0, 0, 0.45), "M_Fabric")
    p.box((0.46, 0.06, 0.55), (0, 0.24, 0.80), "M_Fabric", rot_x=-0.12)
    p.box((0.06, 0.04, 0.2), (0, 0.22, 0.52), "M_MetalDark")
    for x in (-0.26, 0.26):
        p.box((0.04, 0.04, 0.2), (x, 0.05, 0.57), "M_MetalDark")
        p.box((0.06, 0.26, 0.03), (x, 0.03, 0.68), "M_PlasticBlack")
    return p.finish(0.006)


def filing_cabinet(drawers=4):
    h = 0.33 * drawers
    p = Prop(f"SM_FilingCabinet_{drawers}")
    p.box((0.46, 0.62, h), (0, 0, h / 2), "M_MetalLight")
    for i in range(drawers):
        z = 0.33 * i + 0.165
        p.box((0.43, 0.012, 0.30), (0, -0.312, z), "M_MetalLight")
        p.box((0.16, 0.025, 0.025), (0, -0.33, z + 0.07), "M_MetalDark")
        p.box((0.07, 0.006, 0.035), (0, -0.322, z + 0.11), "M_Paper")         # label slot
    return p.finish(0.006)


def filing_shell(drawers=4):
    """Cabinet body with empty drawer bays (the drawers are separate SM_FileDrawer actors that slide out)."""
    h = 0.33 * drawers
    p = Prop(f"SM_FilingShell_{drawers}")
    p.box((0.46, 0.02, h), (0, 0.30, h / 2), "M_MetalLight")                     # back
    for x in (-0.22, 0.22):
        p.box((0.02, 0.62, h), (x, 0, h / 2), "M_MetalLight")                    # sides
    for i in range(drawers + 1):
        p.box((0.46, 0.62, 0.015), (0, 0, min(0.33 * i, h - 0.0075) + 0.0075), "M_MetalLight")   # shelves / top
    return p.finish(0.004)


def file_drawer():
    """One drawer: front panel, handle, label, a tray with hanging folders. Pivot = centre of the front panel's back face."""
    p = Prop("SM_FileDrawer")
    p.box((0.42, 0.012, 0.30), (0, -0.006, 0.0), "M_MetalLight")                # front (faces -Y)
    p.box((0.16, 0.025, 0.025), (0, -0.025, 0.07), "M_MetalDark")              # handle
    p.box((0.07, 0.006, 0.035), (0, -0.015, 0.11), "M_Paper")                  # label slot
    p.box((0.40, 0.56, 0.01), (0, 0.28, -0.13), "M_MetalDark")                 # tray floor
    for x in (-0.2, 0.2):
        p.box((0.01, 0.56, 0.22), (x, 0.28, -0.03), "M_MetalDark")            # tray sides
    for k in range(9):                                                            # hanging folders
        p.box((0.36, 0.01, 0.22), (0, 0.06 + k * 0.055, -0.02), "M_Yellow" if k % 3 else "M_Paper")
    return p.finish(0.002)


def monitor():
    p = Prop("SM_Monitor")
    p.box((0.22, 0.18, 0.015), (0, 0.02, 0.0075), "M_PlasticBlack")
    p.box((0.05, 0.03, 0.28), (0, 0.07, 0.15), "M_PlasticBlack")
    p.box((0.62, 0.035, 0.38), (0, 0.04, 0.38), "M_PlasticBlack")
    p.box((0.59, 0.004, 0.335), (0, 0.021, 0.39), "M_Screen")                 # screen face: own slot, 0..1 UVs for screen images
    return p.finish(0.003, fit_uv={"M_Screen": (-0.295, 0.295, 0.2225, 0.5575)})


def keyboard():
    p = Prop("SM_Keyboard")
    p.box((0.44, 0.14, 0.02), (0, 0, 0.01), "M_PlasticBlack")
    for row in range(5):
        for col in range(14):
            p.box((0.024, 0.022, 0.008), (-0.2 + col * 0.0305, -0.05 + row * 0.025, 0.024), "M_PlasticWhite")
    p.box((0.17, 0.022, 0.008), (0, -0.075, 0.024), "M_PlasticWhite")         # space bar
    return p.finish(0.0015)


def mug():
    p = Prop("SM_Mug")
    p.cyl(0.043, 0.1, (0, 0, 0.05), "M_Ceramic", segs=24)
    p.cyl(0.036, 0.004, (0, 0, 0.098), "M_Wood", segs=24)                      # coffee surface
    p.torus(0.028, 0.008, (0.05, 0, 0.052), "M_Ceramic")
    return p.finish(0.0)


def fire_extinguisher():
    p = Prop("SM_FireExtinguisher")
    p.cyl(0.075, 0.42, (0, 0, 0.21), "M_RedPaint", segs=20)
    p.sphere(0.075, (0, 0, 0.42), "M_RedPaint", segs=20)
    p.cyl(0.02, 0.06, (0, 0, 0.51), "M_MetalDark")
    p.box((0.12, 0.025, 0.015), (0.03, 0, 0.55), "M_MetalDark", rot_y=-0.25)  # lever
    p.cyl(0.012, 0.32, (0.07, 0, 0.36), "M_PlasticBlack", segs=8)             # hose
    p.cyl(0.02, 0.06, (0.07, 0, 0.18), "M_PlasticBlack", segs=8, radius2=0.012)
    p.box((0.1, 0.002, 0.12), (0, -0.075, 0.25), "M_Paper")                    # label
    return p.finish(0.0)


def directors_chair():
    p = Prop("SM_DirectorsChair")
    for side in (-0.26, 0.26):
        for tilt in (0.42, -0.42):
            p.box((0.035, 0.035, 0.95), (side, 0, 0.45), "M_Wood", rot_x=tilt)    # X legs
        p.box((0.035, 0.5, 0.035), (side, 0, 0.66), "M_Wood")                      # arm rails
        p.box((0.035, 0.035, 0.42), (side, 0.23, 0.87), "M_Wood")                  # back posts
        p.box((0.035, 0.5, 0.03), (side, 0, 0.02), "M_Wood")                       # feet rails
    p.box((0.52, 0.42, 0.01), (0, 0, 0.6), "M_Canvas")                             # seat
    p.box((0.52, 0.01, 0.2), (0, 0.24, 0.94), "M_Canvas")                          # back canvas
    return p.finish(0.004)


def water_cooler():
    p = Prop("SM_WaterCooler")
    p.box((0.32, 0.32, 0.95), (0, 0, 0.475), "M_PlasticWhite")
    p.box((0.2, 0.02, 0.12), (0, -0.165, 0.72), "M_PlasticBlack")
    for x, mat in ((-0.05, "M_PlasticBlack"), (0.05, "M_RedPaint")):
        p.box((0.03, 0.04, 0.03), (x, -0.18, 0.76), mat)
    p.cyl(0.13, 0.38, (0, 0, 1.15), "M_Glass", segs=20)
    p.cyl(0.13, 0.08, (0, 0, 1.38), "M_Glass", segs=20, radius2=0.04)
    return p.finish(0.005)


def wet_floor_sign():
    p = Prop("SM_WetFloorSign")
    # An A-frame: the two panels lean IN, so their tops meet under the handle (feet apart, top together)
    for y, tilt in ((-0.12, -0.2), (0.12, 0.2)):
        p.box((0.3, 0.015, 0.62), (0, y, 0.3), "M_Yellow", rot_x=tilt)
    # printed faces (CAUTION / WET FLOOR): thin labels just outside each panel; the back one has its UVs mirrored
    p.box((0.28, 0.002, 0.56), (0, -0.129, 0.3), "M_SignWet", rot_x=-0.2)
    p.box((0.28, 0.002, 0.56), (0, 0.129, 0.3), "M_SignWetBack", rot_x=0.2)
    p.box((0.06, 0.06, 0.03), (0, 0, 0.62), "M_Yellow")
    return p.finish(0.006, fit_uv={"M_SignWet": (-0.14, 0.14, 0.02, 0.58), "M_SignWetBack": (0.14, -0.14, 0.02, 0.58)})


def cubicle_panel():
    p = Prop("SM_CubiclePanel")
    p.box((1.5, 0.05, 1.36), (0, 0, 0.7), "M_FabricMint")
    p.box((1.52, 0.06, 0.03), (0, 0, 1.395), "M_MetalLight")
    for x in (-0.75, 0.75):
        p.box((0.03, 0.06, 1.4), (x, 0, 0.7), "M_MetalLight")
    for x in (-0.6, 0.6):
        p.box((0.05, 0.3, 0.02), (x, 0, 0.01), "M_MetalLight")                  # feet
    return p.finish(0.004)


def ream_of_paper():
    p = Prop("SM_ReamOfPaper")
    p.box((0.297, 0.21, 0.05), (0, 0, 0.025), "M_Paper")
    p.box((0.299, 0.08, 0.052), (0, 0, 0.025), "M_FabricMint")               # wrapper band
    return p.finish(0.002)


def hospital_bed():
    p = Prop("SM_HospitalBed")
    p.box((0.9, 2.0, 0.08), (0, 0, 0.5), "M_MetalLight")
    p.box((0.85, 1.95, 0.15), (0, 0, 0.615), "M_Mattress")
    p.box((0.6, 0.35, 0.08), (0, 0.75, 0.73), "M_Mattress")                   # pillow
    p.box((0.9, 0.04, 0.5), (0, 1.0, 0.75), "M_PlasticWhite")                  # headboard
    p.box((0.9, 0.04, 0.35), (0, -1.0, 0.68), "M_PlasticWhite")                # footboard
    for x in (-0.45, 0.45):
        p.box((0.025, 1.2, 0.025), (x, 0.1, 0.82), "M_MetalLight")             # side rails
        for y in (-0.9, 0.9):
            p.box((0.04, 0.04, 0.42), (x * 0.9, y, 0.27), "M_MetalLight")
            p.cyl(0.05, 0.03, (x * 0.9, y, 0.05), "M_PlasticBlack", segs=12, axis="X")
    return p.finish(0.006)


def mop_bucket():
    p = Prop("SM_MopBucket")
    p.cyl(0.2, 0.34, (0, 0, 0.19), "M_Yellow", segs=20, radius2=0.22)
    p.cyl(0.19, 0.01, (0, 0, 0.3), "M_Glass", segs=20)                         # dirty water
    p.box((0.16, 0.24, 0.16), (0.1, 0, 0.42), "M_Yellow")                      # wringer
    for x, y in ((-0.15, -0.15), (0.15, -0.15), (-0.15, 0.15), (0.15, 0.15)):
        p.sphere(0.022, (x, y, 0.022), "M_PlasticBlack", segs=8)
    p.cyl(0.012, 1.2, (-0.05, 0.05, 0.75), "M_Wood", segs=8)                   # mop handle
    return p.finish(0.004)


def coffee_machine():
    p = Prop("SM_CoffeeMachine")
    p.box((0.3, 0.35, 0.42), (0, 0.03, 0.21), "M_PlasticBlack")
    p.box((0.26, 0.04, 0.12), (0, -0.16, 0.34), "M_MetalLight")
    p.box((0.2, 0.2, 0.02), (0, -0.07, 0.01), "M_MetalDark")
    p.cyl(0.075, 0.16, (0, -0.08, 0.1), "M_Glass", segs=20)                    # carafe
    p.box((0.04, 0.01, 0.02), (0.08, -0.18, 0.37), "M_RedPaint")               # power light
    return p.finish(0.004)


# --------------------------------------------------------------------------
# Office supplies (most are throwable)
# --------------------------------------------------------------------------
def binder(colour="M_BinderBlue", name="SM_Binder"):
    p = Prop(name)
    p.box((0.05, 0.28, 0.32), (0, 0, 0.16), colour)
    p.box((0.042, 0.27, 0.305), (0.006, 0, 0.16), "M_Paper")
    p.cyl(0.012, 0.012, (-0.026, 0.0, 0.025), "M_MetalLight", segs=10, axis="X")  # spine ring hole
    p.box((0.002, 0.18, 0.06), (-0.026, 0, 0.24), "M_Paper")                        # spine label
    return p.finish(0.003)


def stapler():
    p = Prop("SM_Stapler")
    p.box((0.05, 0.17, 0.02), (0, 0, 0.01), "M_PlasticBlack")
    p.box((0.042, 0.16, 0.03), (0, 0.004, 0.038), "M_MetalDark", rot_x=0.06)
    return p.finish(0.003)


def tape_dispenser():
    p = Prop("SM_TapeDispenser")
    p.box((0.06, 0.14, 0.05), (0, 0, 0.025), "M_PlasticBlack")
    p.cyl(0.03, 0.02, (0, 0.01, 0.07), "M_Glass", segs=16, axis="X")
    return p.finish(0.004)


def pen_cup():
    p = Prop("SM_PenCup")
    p.cyl(0.04, 0.1, (0, 0, 0.05), "M_MetalDark", segs=16)
    for i, (dx, dy, col) in enumerate(((0.01, 0.01, "M_BinderBlue"), (-0.015, 0.0, "M_BinderRed"),
                                       (0.0, -0.015, "M_PlasticBlack"), (0.018, -0.01, "M_Yellow"))):
        p.cyl(0.004, 0.15, (dx, dy, 0.1 + i * 0.006), col, segs=6)
    return p.finish(0.0)


def desk_lamp():
    p = Prop("SM_DeskLamp")
    p.cyl(0.07, 0.02, (0, 0, 0.01), "M_PlasticWhite", segs=20)
    p.box((0.015, 0.015, 0.3), (0, 0, 0.16), "M_PlasticWhite", rot_y=0.25)
    p.box((0.015, 0.015, 0.25), (0.11, 0, 0.36), "M_PlasticWhite", rot_y=1.0)
    p.cyl(0.06, 0.09, (0.22, 0, 0.40), "M_PlasticWhite", segs=16, radius2=0.025)
    return p.finish(0.003)


def desk_phone():
    p = Prop("SM_DeskPhone")
    p.box((0.2, 0.18, 0.05), (0, 0, 0.025), "M_PlasticBlack", rot_x=-0.15)
    p.box((0.05, 0.2, 0.035), (-0.07, 0, 0.07), "M_PlasticBlack")                    # handset
    p.box((0.08, 0.06, 0.003), (0.04, -0.02, 0.054), "M_Screen")
    return p.finish(0.004)


def pc_tower():
    p = Prop("SM_PCTower")
    p.box((0.2, 0.45, 0.42), (0, 0, 0.21), "M_PlasticBlack")
    p.box((0.16, 0.005, 0.03), (0, -0.226, 0.37), "M_MetalLight")
    p.cyl(0.012, 0.006, (0.05, -0.227, 0.33), "M_SkyBlue", segs=12, axis="Y")       # power LED
    return p.finish(0.004)


def laptop():
    p = Prop("SM_Laptop")
    p.box((0.33, 0.23, 0.018), (0, 0, 0.009), "M_MetalLight")
    p.box((0.33, 0.012, 0.22), (0, 0.12, 0.115), "M_MetalLight", rot_x=-0.25)
    p.box((0.30, 0.004, 0.19), (0, 0.112, 0.118), "M_Screen", rot_x=-0.25)
    return p.finish(0.003)


def printer():
    p = Prop("SM_Printer")
    p.box((0.6, 0.55, 0.9), (0, 0, 0.45), "M_PlasticWhite")                          # copier body
    p.box((0.6, 0.55, 0.12), (0, 0, 0.96), "M_PlasticWhite")
    p.box((0.4, 0.2, 0.004), (0.05, -0.12, 1.022), "M_Screen")                      # glass
    p.box((0.18, 0.08, 0.05), (-0.18, -0.24, 1.0), "M_MetalDark")                    # panel
    for z in (0.2, 0.42):
        p.box((0.56, 0.01, 0.18), (0, -0.276, z), "M_MetalLight")
    p.box((0.3, 0.25, 0.04), (0.42, 0, 0.75), "M_PlasticWhite")                      # output tray
    return p.finish(0.006)


def trash_bin():
    p = Prop("SM_TrashBin")
    p.cyl(0.14, 0.32, (0, 0, 0.16), "M_MetalDark", segs=20, radius2=0.16)
    p.box((0.08, 0.06, 0.05), (0.02, 0.0, 0.33), "M_Paper", rot_z=0.5)             # crumpled paper
    return p.finish(0.0)


def paper_ball():
    """A crumpled sheet: three squashed lumps of paper (for the paper toss)."""
    p = Prop("SM_PaperBall")
    p.sphere(0.035, (0, 0, 0.035), "M_Paper", segs=8)
    p.sphere(0.024, (0.018, 0.01, 0.045), "M_Paper", segs=6)
    p.sphere(0.022, (-0.015, -0.012, 0.028), "M_Paper", segs=6)
    return p.finish(0.0)


def wall_clock():
    p = Prop("SM_WallClock")   # pivot at its back centre; hang on walls. Hands frozen at 2:14.
    p.cyl(0.16, 0.04, (0, 0.02, 0), "M_PlasticWhite", segs=32, axis="Y")
    p.cyl(0.145, 0.002, (0, -0.001, 0), "M_ClockFace", segs=32, axis="Y")
    # Hands start at the centre and point at the hour: a box's long (Z) side turned by +a about Y points at
    # (sin a, cos a) in (X, Z), i.e. a degrees clockwise from 12 seen from the front (-Y); its centre goes half a length out.
    def hand(length, width, depth_y, angle_deg, mat):
        a = math.radians(angle_deg)
        p.box((width, 0.004, length), (math.sin(a) * length / 2, depth_y, math.cos(a) * length / 2), mat, rot_y=a)
    hand(0.075, 0.013, -0.005, 67, "M_PlasticBlack")     # hour: 2:14 = 2 * 30 + 14 * 0.5 = 67 degrees
    hand(0.115, 0.008, -0.009, 84, "M_PlasticBlack")     # minute: 14 * 6 = 84 degrees
    hand(0.125, 0.003, -0.013, 0, "M_RedPaint")          # second hand, stopped at 12
    p.cyl(0.01, 0.01, (0, -0.016, 0), "M_RedPaint", segs=8, axis="Y")
    return p.finish(0.0)


def pass_reader():
    p = Prop("SM_PassReader")  # wall-mounted; pivot at back centre
    p.box((0.09, 0.03, 0.14), (0, 0.015, 0), "M_PlasticBlack")
    p.box((0.06, 0.004, 0.04), (0, -0.001, 0.03), "M_Screen")
    p.cyl(0.006, 0.004, (0, -0.002, -0.04), "M_ExitGreen", segs=8, axis="Y")
    return p.finish(0.003)


def exit_sign():
    p = Prop("SM_ExitSign")
    p.box((0.4, 0.06, 0.18), (0, 0, 0), "M_PlasticWhite")
    p.box((0.34, 0.004, 0.12), (0, -0.032, 0), "M_ExitFace")
    return p.finish(0.003, fit_uv={"M_ExitFace": (-0.17, 0.17, -0.06, 0.06)})


# --------------------------------------------------------------------------
# Bright corporate design: plants, lobby, signage
# --------------------------------------------------------------------------
def _leaves(p, centre, count, spread, size, seed, mats=("M_Leaf", "M_LeafLight")):
    import random
    rnd = random.Random(seed)
    golden = math.pi * (3 - math.sqrt(5))
    for i in range(count):
        a = i * golden
        r = spread * math.sqrt((i + 0.5) / count)
        h = rnd.uniform(-0.4, 0.4) * spread
        loc = (centre[0] + math.cos(a) * r, centre[1] + math.sin(a) * r, centre[2] + h)
        p.box((size, size * 0.45, size * 0.06), loc, mats[i % 2], rot_z=a, rot_x=rnd.uniform(-0.6, 0.6),
              rot_y=rnd.uniform(-0.5, 0.5))


def plant_tall():
    p = Prop("SM_PlantTall")   # fake ficus in a white pot
    p.cyl(0.2, 0.42, (0, 0, 0.21), "M_PotWhite", segs=24, radius2=0.24)
    p.cyl(0.21, 0.02, (0, 0, 0.42), "M_Soil", segs=24)
    p.cyl(0.025, 1.1, (0, 0, 0.95), "M_Wood", segs=8)
    _leaves(p, (0, 0, 1.45), 70, 0.38, 0.13, 7)
    _leaves(p, (0, 0, 1.05), 30, 0.28, 0.12, 9)
    return p.finish(0.0)


def plant_desk():
    p = Prop("SM_PlantDesk")   # small succulent, throwable
    p.cyl(0.06, 0.08, (0, 0, 0.04), "M_PotWhite", segs=16, radius2=0.07)
    _leaves(p, (0, 0, 0.11), 14, 0.05, 0.06, 3)
    return p.finish(0.0)


def planter_box():
    p = Prop("SM_PlanterBox")  # long low planter for the lobby
    p.box((1.6, 0.4, 0.45), (0, 0, 0.225), "M_PotWhite")
    p.box((1.55, 0.36, 0.02), (0, 0, 0.44), "M_Soil")
    for i, x in enumerate((-0.6, -0.2, 0.2, 0.6)):
        _leaves(p, (x, 0, 0.65), 22, 0.18, 0.11, 20 + i)
    return p.finish(0.0)


def lobby_sofa():
    p = Prop("SM_LobbySofa")
    p.box((2.0, 0.85, 0.42), (0, 0, 0.21), "M_SofaMint")
    p.box((2.0, 0.2, 0.45), (0, 0.33, 0.62), "M_SofaMint")
    for x in (-0.95, 0.95):
        p.box((0.12, 0.85, 0.25), (x, 0, 0.53), "M_SofaMint")
    for x in (-0.48, 0.48):
        p.box((0.94, 0.62, 0.12), (x, -0.08, 0.48), "M_SofaMint")                     # cushions
    return p.finish(0.02)


def coffee_table():
    p = Prop("SM_CoffeeTable")
    p.cyl(0.45, 0.04, (0, 0, 0.42), "M_Wood", segs=32)
    p.cyl(0.05, 0.4, (0, 0, 0.2), "M_MetalLight", segs=12)
    p.cyl(0.25, 0.02, (0, 0, 0.01), "M_MetalLight", segs=24)
    return p.finish(0.004)


def logo_sign():
    p = Prop("SM_HalcyonSign")  # wall logo: half-sun over horizon lines; pivot at back centre
    segs = 14
    for i in range(segs):
        a0, a1 = math.pi * i / segs, math.pi * (i + 1) / segs
        mid = (a0 + a1) / 2
        p.box((0.1, 0.05, 0.42 * (a1 - a0) * 1.1), (math.cos(mid) * 0.42, 0, 0.1 + math.sin(mid) * 0.42),
              "M_FabricMint", rot_y=-mid)
    p.box((1.1, 0.05, 0.06), (0, 0, 0.0), "M_FabricMint")
    p.box((0.7, 0.05, 0.05), (0, 0, -0.12), "M_FabricMint")
    return p.finish(0.004)


def meeting_chair():
    p = Prop("SM_MeetingChair")
    for x in (-0.2, 0.2):
        for y in (-0.2, 0.2):
            p.cyl(0.012, 0.45, (x, y, 0.225), "M_MetalLight", segs=8)
    p.box((0.46, 0.46, 0.05), (0, 0, 0.47), "M_SofaMint")
    p.box((0.46, 0.04, 0.4), (0, 0.22, 0.72), "M_SofaMint", rot_x=-0.1)
    return p.finish(0.008)


# --------------------------------------------------------------------------
# Kitchen
# --------------------------------------------------------------------------
def kitchen_counter():
    p = Prop("SM_KitchenCounter")   # 2.4 m run with sink; pivot floor-centre, back against +Y
    p.box((2.4, 0.6, 0.86), (0, 0, 0.43), "M_PlasticWhite")
    p.box((2.44, 0.64, 0.04), (0, -0.01, 0.88), "M_Counter")
    for i in range(4):
        x = -0.9 + i * 0.6
        p.box((0.57, 0.01, 0.7), (x, -0.305, 0.43), "M_PlasticWhite")
        p.box((0.12, 0.02, 0.015), (x, -0.315, 0.74), "M_MetalLight")
    p.box((0.5, 0.4, 0.015), (0.5, 0.0, 0.903), "M_Steel")                          # sink
    p.cyl(0.012, 0.25, (0.5, 0.2, 1.02), "M_Steel", segs=8)                        # tap
    p.box((0.015, 0.15, 0.015), (0.5, 0.13, 1.14), "M_Steel")
    p.box((2.4, 0.35, 0.7), (0, 0.125, 1.85), "M_PlasticWhite")                    # wall cabinets
    return p.finish(0.005)


def kitchen_counter_short():
    p = Prop("SM_KitchenCounterShort")   # 1.2 m base run, no sink; fills the gap next to the fridge
    p.box((1.2, 0.6, 0.86), (0, 0, 0.43), "M_PlasticWhite")
    p.box((1.24, 0.64, 0.04), (0, -0.01, 0.88), "M_Counter")
    for i in range(2):
        x = -0.3 + i * 0.6
        p.box((0.57, 0.01, 0.7), (x, -0.305, 0.43), "M_PlasticWhite")
        p.box((0.12, 0.02, 0.015), (x, -0.315, 0.74), "M_MetalLight")
    p.box((1.2, 0.35, 0.7), (0, 0.125, 1.85), "M_PlasticWhite")                    # wall cabinets
    return p.finish(0.005)


def kitchen_island():
    p = Prop("SM_KitchenIsland")   # 1.8 x 0.9 m island, worktop at 90 cm; pivot floor-centre
    p.box((1.7, 0.8, 0.86), (0, 0, 0.43), "M_PlasticWhite")
    p.box((1.86, 0.96, 0.04), (0, 0, 0.88), "M_Counter")
    p.box((1.72, 0.82, 0.08), (0, 0, 0.04), "M_MetalDark")                         # kick plate
    for i in range(3):
        x = -0.57 + i * 0.57
        p.box((0.54, 0.01, 0.7), (x, -0.405, 0.46), "M_PlasticWhite")
        p.box((0.12, 0.02, 0.015), (x, -0.415, 0.76), "M_MetalLight")
    return p.finish(0.005)


def moving_box():
    p = Prop("SM_MovingBox")   # 46 x 34 x 30 cm cardboard box, flaps open, someone's desk things inside
    p.box((0.46, 0.34, 0.30), (0, 0, 0.15), "M_Cardboard")
    p.box((0.44, 0.32, 0.01), (0, 0, 0.295), "M_PlasticBlack")                    # dark inside
    p.box((0.46, 0.01, 0.16), (0, -0.20, 0.36), "M_Cardboard", rot_x=math.radians(-35))   # front flap, open
    p.box((0.46, 0.01, 0.16), (0, 0.20, 0.36), "M_Cardboard", rot_x=math.radians(35))     # back flap
    p.box((0.08, 0.10, 0.14), (-0.12, 0.05, 0.33), "M_Ceramic")                   # a mug
    p.box((0.18, 0.02, 0.22), (0.08, 0.06, 0.38), "M_MetalDark", rot_x=math.radians(-10)) # a picture frame
    p.box((0.30, 0.24, 0.04), (0.02, -0.02, 0.32), "M_Paper")                     # papers
    return p.finish(0.003)


def fridge():
    p = Prop("SM_Fridge")
    p.box((0.7, 0.7, 1.8), (0, 0, 0.9), "M_Steel")
    p.box((0.68, 0.01, 0.01), (0, -0.351, 1.2), "M_MetalDark")
    for z in (1.5, 0.8):
        p.box((0.02, 0.03, 0.35), (0.3, -0.37, z), "M_MetalDark")
    p.box((0.12, 0.003, 0.16), (-0.15, -0.352, 1.45), "M_Paper", rot_y=0.08)       # a memo on the door
    return p.finish(0.008)


def microwave():
    p = Prop("SM_Microwave")
    p.box((0.5, 0.38, 0.29), (0, 0, 0.145), "M_Steel")
    p.box((0.33, 0.005, 0.22), (-0.06, -0.19, 0.15), "M_Screen")
    p.box((0.1, 0.006, 0.22), (0.18, -0.19, 0.15), "M_PlasticBlack")
    return p.finish(0.004)


def kettle():
    p = Prop("SM_Kettle")
    p.cyl(0.08, 0.02, (0, 0, 0.01), "M_PlasticBlack", segs=20)
    p.cyl(0.075, 0.2, (0, 0, 0.12), "M_PlasticWhite", segs=20, radius2=0.06)
    p.box((0.03, 0.03, 0.14), (0.085, 0, 0.13), "M_PlasticWhite")
    p.box((0.05, 0.02, 0.02), (-0.085, 0, 0.17), "M_PlasticWhite", rot_y=0.5)
    return p.finish(0.003)


def vending_machine():
    p = Prop("SM_VendingMachine")
    p.box((0.9, 0.8, 1.85), (0, 0, 0.925), "M_FabricMint")
    p.box((0.62, 0.01, 1.2), (-0.1, -0.405, 1.1), "M_Glass")
    for r in range(5):
        for c in range(5):
            p.box((0.08, 0.06, 0.14), (-0.34 + c * 0.12, -0.33, 0.6 + r * 0.22),
                  ("M_BinderRed", "M_Yellow", "M_BinderBlue", "M_LeafLight", "M_PlasticWhite")[(r + c) % 5])
    p.box((0.16, 0.01, 0.4), (0.33, -0.405, 1.2), "M_PlasticBlack")
    p.box((0.6, 0.01, 0.15), (-0.1, -0.405, 0.3), "M_PlasticBlack")
    return p.finish(0.008)


def cafe_table():
    p = Prop("SM_CafeTable")
    p.cyl(0.4, 0.03, (0, 0, 1.03), "M_Counter", segs=28)
    p.cyl(0.03, 1.0, (0, 0, 0.52), "M_Steel", segs=10)
    p.cyl(0.25, 0.02, (0, 0, 0.01), "M_Steel", segs=24)
    return p.finish(0.004)


def bar_stool():
    p = Prop("SM_BarStool")
    p.cyl(0.18, 0.06, (0, 0, 0.75), "M_SofaMint", segs=20)
    p.cyl(0.025, 0.72, (0, 0, 0.36), "M_Steel", segs=10)
    p.cyl(0.2, 0.02, (0, 0, 0.01), "M_Steel", segs=20)
    p.cyl(0.16, 0.015, (0, 0, 0.3), "M_Steel", segs=20)                             # foot ring
    return p.finish(0.004)


# --------------------------------------------------------------------------
# Figures: mannequin coworkers + the stalker
# --------------------------------------------------------------------------
def _figure(p, height, skin, suit, thin=1.0):
    s = height / 1.8
    p.sphere(0.11 * s, (0, 0, 1.66 * s), skin, segs=16)                            # head, featureless
    p.cyl(0.05 * s * thin, 0.1 * s, (0, 0, 1.52 * s), skin, segs=10)                # neck
    p.box((0.42 * s * thin, 0.22 * s, 0.6 * s), (0, 0, 1.18 * s), suit)             # torso
    p.box((0.38 * s * thin, 0.2 * s, 0.18 * s), (0, 0, 0.84 * s), suit)             # hips
    for side in (-1, 1):
        p.cyl(0.05 * s * thin, 0.6 * s, (side * 0.27 * s * thin, 0, 1.12 * s), suit, segs=10)   # arms
        p.sphere(0.045 * s, (side * 0.27 * s * thin, 0, 0.8 * s), skin, segs=10)               # hands
        p.cyl(0.065 * s * thin, 0.8 * s, (side * 0.1 * s * thin, 0, 0.4 * s), suit, segs=10)  # legs
        p.box((0.1 * s, 0.24 * s, 0.06 * s), (side * 0.1 * s * thin, -0.05 * s, 0.03 * s), "M_PlasticBlack")


def mannequin():
    p = Prop("SM_Mannequin")       # coworkers: smooth, faceless, in HALCYON mint
    _figure(p, 1.78, "M_Skin", "M_FabricMint")
    p.box((0.08, 0.005, 0.11), (0.1, -0.115, 1.28), "M_PlasticWhite")              # badge
    return p.finish(0.0)


def mannequin_guard():
    p = Prop("SM_MannequinGuard")
    _figure(p, 1.85, "M_Skin", "M_BinderBlue")
    p.cyl(0.12, 0.05, (0, 0, 1.78), "M_MetalDark", segs=16)                          # cap
    return p.finish(0.0)


def mannequin_custodian():
    p = Prop("SM_MannequinCustodian")
    _figure(p, 1.72, "M_Skin", "M_Yellow")
    return p.finish(0.0)


def stalker():
    p = Prop("SM_Stalker")         # too tall, too thin, in a dark suit
    _figure(p, 2.35, "M_StalkerSkin", "M_MetalDark", thin=0.72)
    return p.finish(0.0)


# --------------------------------------------------------------------------
# Wishlist batch: reception desk, fridge (body + hinged door), doors, kitchen stock
# --------------------------------------------------------------------------
def reception_desk():
    p = Prop("SM_ReceptionDesk")   # front (logo panel) faces -Y; 2.8 m wide
    p.box((2.8, 0.75, 1.05), (0, 0, 0.525), "M_PlasticWhite")
    p.box((2.9, 0.85, 0.04), (0, 0, 1.07), "M_Wood")                                # top
    p.box((2.4, 0.5, 0.04), (0, 0.15, 0.76), "M_DeskTop")                          # inner work surface
    p.box((2.82, 0.02, 0.12), (0, -0.385, 0.08), "M_FabricMint")                   # glowing kick strip
    return p.finish(0.01)


def fridge_body():
    p = Prop("SM_FridgeBody")      # open front faces -Y; shelves inside
    w, d, h = 0.7, 0.7, 1.8
    p.box((w, d, 0.04), (0, 0, 0.02), "M_Steel")
    p.box((w, d, 0.04), (0, 0, h - 0.02), "M_Steel")
    for x in (-w / 2 + 0.02, w / 2 - 0.02):
        p.box((0.04, d, h), (x, 0, h / 2), "M_Steel")
    p.box((w, 0.04, h), (0, d / 2 - 0.02, h / 2), "M_Steel")
    # liner: thin panels (it used to be one solid box that filled the inside, so the fridge looked like a white wall)
    p.box((w - 0.08, 0.01, h - 0.1), (0, d / 2 - 0.045, h / 2), "M_PlasticWhite")     # back
    for x in (-w / 2 + 0.045, w / 2 - 0.045):
        p.box((0.01, d - 0.08, h - 0.1), (x, 0, h / 2), "M_PlasticWhite")            # sides
    for z in (0.45, 0.85, 1.25):
        p.box((w - 0.1, d - 0.12, 0.012), (0, -0.02, z), "M_Glass")              # glass shelves
    p.box((w - 0.08, 0.01, 0.6), (0, -d / 2 + 0.06, 1.45), "M_PlasticWhite")    # freezer divider lip
    return p.finish(0.004)


def fridge_door():
    p = Prop("SM_FridgeDoor")      # PIVOT ON THE HINGE (left edge, seen from the front); door lies along +X
    p.box((0.7, 0.05, 1.78), (0.35, -0.025, 0.9), "M_Steel")
    p.box((0.02, 0.03, 0.4), (0.64, -0.065, 1.15), "M_MetalDark")                  # handle
    for z in (0.5, 0.9, 1.3):
        p.box((0.6, 0.08, 0.06), (0.35, 0.05, z), "M_PlasticWhite")              # door bins (inside)
    return p.finish(0.004)


def door():
    p = Prop("SM_Door")            # 0.9 x 2.1 m, PIVOT ON THE HINGE; door lies along +X
    p.box((0.9, 0.045, 2.1), (0.45, 0, 1.05), "M_PlasticWhite")
    p.box((0.03, 0.06, 0.03), (0.8, -0.04, 1.0), "M_MetalLight")                  # handle front
    p.box((0.03, 0.06, 0.03), (0.8, 0.04, 1.0), "M_MetalLight")                   # handle back
    p.box((0.24, 0.002, 0.07), (0.45, -0.024, 1.55), "M_Paper")                    # name plate
    return p.finish(0.003)


def door_frame():
    p = Prop("SM_DoorFrame")       # fits a 0.9 m door; pivot floor centre
    for x in (-0.48, 0.48):
        p.box((0.06, 0.16, 2.16), (x, 0, 1.08), "M_MetalLight")
    p.box((1.02, 0.16, 0.06), (0, 0, 2.16), "M_MetalLight")
    return p.finish(0.003)


def utility_shelf():
    p = Prop("SM_UtilityShelf")    # janitor closet / supply shelving
    for x in (-0.58, 0.58):
        for y in (-0.18, 0.18):
            p.box((0.03, 0.03, 1.9), (x, y, 0.95), "M_MetalLight")
    for z in (0.15, 0.6, 1.05, 1.5, 1.88):
        p.box((1.2, 0.4, 0.02), (0, 0, z), "M_MetalLight")
    return p.finish(0.002)


def coffee_bag():
    p = Prop("SM_CoffeeBag")
    p.box((0.12, 0.07, 0.2), (0, 0, 0.1), "M_Wood")
    p.box((0.121, 0.071, 0.05), (0, 0, 0.12), "M_FabricMint")                      # HALCYON band
    p.box((0.1, 0.05, 0.03), (0, 0, 0.215), "M_Wood", rot_x=0.3)                  # folded top
    return p.finish(0.003)


def tea_box():
    p = Prop("SM_TeaBox")
    p.box((0.16, 0.08, 0.07), (0, 0, 0.035), "M_LeafLight")
    p.box((0.161, 0.081, 0.02), (0, 0, 0.05), "M_PlasticWhite")
    return p.finish(0.002)


def cup_stack():
    p = Prop("SM_CupStack")        # stacked paper cups
    for i in range(8):
        p.cyl(0.04, 0.03, (0, 0, 0.015 + i * 0.012), "M_Paper", segs=16, radius2=0.045)
    p.cyl(0.045, 0.012, (0, 0, 0.11), "M_FabricMint", segs=16)
    return p.finish(0.0)


def snack_bag():
    p = Prop("SM_SnackBag")
    p.box((0.16, 0.05, 0.22), (0, 0, 0.11), "M_BinderRed")
    p.box((0.162, 0.03, 0.02), (0, 0, 0.225), "M_BinderRed")                       # crimp
    p.box((0.162, 0.051, 0.06), (0, 0, 0.1), "M_Yellow")
    return p.finish(0.004)


def snack_box():
    p = Prop("SM_SnackBox")        # cereal / granola bar box
    p.box((0.2, 0.07, 0.28), (0, 0, 0.14), "M_Yellow")
    p.box((0.201, 0.071, 0.08), (0, 0, 0.2), "M_BinderBlue")
    return p.finish(0.003)


def soda_can():
    p = Prop("SM_SodaCan")
    p.cyl(0.033, 0.12, (0, 0, 0.06), "M_SkyBlue", segs=16)
    p.cyl(0.03, 0.005, (0, 0, 0.122), "M_MetalLight", segs=16)
    return p.finish(0.0)


def water_bottle():
    p = Prop("SM_WaterBottle")
    p.cyl(0.035, 0.18, (0, 0, 0.09), "M_Glass", segs=16)
    p.cyl(0.035, 0.04, (0, 0, 0.2), "M_Glass", segs=16, radius2=0.014)
    p.cyl(0.015, 0.02, (0, 0, 0.23), "M_FabricMint", segs=10)
    return p.finish(0.0)


def milk_carton():
    p = Prop("SM_MilkCarton")
    p.box((0.09, 0.09, 0.2), (0, 0, 0.1), "M_PlasticWhite")
    p.box((0.09, 0.02, 0.06), (0, 0, 0.22), "M_PlasticWhite", rot_x=0.6)
    p.box((0.091, 0.091, 0.05), (0, 0, 0.12), "M_SkyBlue")
    return p.finish(0.002)


def lunch_box():
    p = Prop("SM_LunchBox")        # labelled 4471 on the note in game
    p.box((0.24, 0.16, 0.1), (0, 0, 0.05), "M_BinderMint")
    p.box((0.08, 0.02, 0.02), (0, 0, 0.11), "M_PlasticBlack")
    p.box((0.1, 0.002, 0.04), (0, -0.081, 0.05), "M_Paper")
    return p.finish(0.004)


def sandwich():
    """Lunch (Act 2): a stacked sandwich on a paper napkin."""
    p = Prop("SM_Sandwich")
    p.box((0.16, 0.16, 0.002), (0, 0, 0.001), "M_Paper")
    p.box((0.12, 0.10, 0.016), (0, 0, 0.010), "M_Bread")
    p.box((0.128, 0.108, 0.006), (0, 0, 0.021), "M_LeafLight")
    p.box((0.118, 0.098, 0.006), (0, 0, 0.027), "M_Cheese")
    p.box((0.110, 0.090, 0.008), (0, 0, 0.034), "M_Tomato")
    p.box((0.12, 0.10, 0.016), (0, 0, 0.046), "M_Bread")
    return p.finish(0.003)


def pizza_slice():
    """Lunch (Act 2): one slice on a paper plate (a triangle from a 3-sided cylinder)."""
    p = Prop("SM_PizzaSlice")
    p.cyl(0.11, 0.004, (0, 0, 0.002), "M_Paper", segs=24)
    p.cyl(0.085, 0.012, (0, 0, 0.010), "M_Cheese", segs=3)
    p.box((0.13, 0.03, 0.018), (0.0, 0.045, 0.012), "M_Crust")
    for (x, y) in ((-0.01, -0.01), (0.02, 0.0), (0.0, -0.035)):
        p.cyl(0.012, 0.004, (x, y, 0.017), "M_Tomato", segs=10)
    return p.finish(0.0)


def salad_bowl():
    p = Prop("SM_SaladBowl")
    p.cyl(0.09, 0.06, (0, 0, 0.03), "M_Ceramic", segs=24, radius2=0.065)
    for i, (dx, dy) in enumerate(((0.03, 0.0), (-0.03, 0.02), (0.0, -0.03), (-0.01, 0.0), (0.02, 0.03))):
        p.sphere(0.035, (dx, dy, 0.065), "M_LeafLight" if i % 2 else "M_Leaf", segs=8)
    p.sphere(0.014, (0.01, -0.01, 0.09), "M_Tomato", segs=8)
    return p.finish(0.0)


def donut():
    """A ring of lumps: dough underneath, pink frosting on top."""
    import math as _m
    p = Prop("SM_Donut")
    for i in range(12):
        a = 2 * _m.pi * i / 12
        p.sphere(0.022, (_m.cos(a) * 0.035, _m.sin(a) * 0.035, 0.02), "M_Bread", segs=8)
        p.sphere(0.018, (_m.cos(a) * 0.035, _m.sin(a) * 0.035, 0.03), "M_Frosting", segs=8)
    return p.finish(0.0)


# --- restroom (Saturday) ----------------------------------------------------
def toilet():
    p = Prop("SM_Toilet")             # faces -Y (front = the bowl)
    p.box((0.4, 0.18, 0.4), (0, 0.2, 0.6), "M_Ceramic")                         # tank
    p.cyl(0.19, 0.38, (0, -0.06, 0.19), "M_Ceramic", segs=20, radius2=0.16)      # bowl
    p.cyl(0.2, 0.03, (0, -0.06, 0.395), "M_PlasticWhite", segs=20)              # seat
    p.box((0.06, 0.02, 0.02), (0.14, 0.11, 0.78), "M_MetalLight")                # flush handle
    return p.finish(0.004)


def sink():
    p = Prop("SM_Sink")               # counter + basin; faces -Y
    p.box((0.6, 0.45, 0.06), (0, 0, 0.82), "M_Counter")
    p.box((0.6, 0.45, 0.76), (0, 0.0, 0.38), "M_PlasticWhite")
    p.cyl(0.17, 0.05, (0, -0.02, 0.86), "M_Ceramic", segs=20)
    p.cyl(0.015, 0.16, (0, 0.17, 0.92), "M_Steel", segs=10)                      # tap
    p.box((0.03, 0.12, 0.025), (0, 0.11, 0.99), "M_Steel")
    return p.finish(0.004)


def mirror():
    p = Prop("SM_Mirror")             # 0.55 x 0.75, hangs on a wall; pivot at the bottom centre of the back
    p.box((0.59, 0.02, 0.79), (0, 0, 0.395), "M_Steel")
    p.box((0.55, 0.004, 0.75), (0, -0.012, 0.395), "M_Mirror")
    return p.finish(0.002)


def hand_dryer():
    p = Prop("SM_HandDryer")
    p.box((0.28, 0.16, 0.3), (0, 0, 0.15), "M_PlasticWhite")
    p.box((0.16, 0.08, 0.04), (0, -0.04, 0.0), "M_Steel")                       # nozzle
    p.box((0.08, 0.002, 0.03), (0, -0.081, 0.22), "M_ExitGreen")                 # little light
    return p.finish(0.006)


def stall_door():
    p = Prop("SM_StallDoor")          # 0.62 x 1.5 m, raised 0.25 m (feet show underneath); PIVOT ON THE HINGE along +X
    p.box((0.62, 0.03, 1.5), (0.31, 0, 1.0), "M_FabricMint")
    p.box((0.04, 0.04, 0.04), (0.55, -0.03, 1.05), "M_Steel")
    p.box((0.05, 0.002, 0.02), (0.55, -0.051, 1.12), "M_RedPaint")              # OCCUPIED
    return p.finish(0.003)


def shoes():
    p = Prop("SM_Shoes")              # a pair of black office shoes, toes toward -Y
    for x in (-0.07, 0.07):
        p.box((0.09, 0.26, 0.07), (x, 0, 0.035), "M_PlasticBlack")
        p.box((0.07, 0.06, 0.2), (x, 0.09, 0.13), "M_Fabric")                   # ankles
    return p.finish(0.01)


def sign_plaque():
    p = Prop("SM_SignPlaque")         # 0.3 x 0.3 wall plaque; face -Y gets a texture (SlotMI=M_SignFace:...)
    p.box((0.3, 0.015, 0.3), (0, 0, 0.15), "M_PlasticWhite")
    p.box((0.28, 0.002, 0.28), (0, -0.009, 0.15), "M_SignFace")
    return p.finish(0.003, fit_uv={"M_SignFace": (-0.14, 0.14, 0.01, 0.29)})


def security_cam():
    """Wall camera. Pivot = the wall mount; the lens looks along -Y (Unreal +Y). C++ turns the whole actor to follow you."""
    p = Prop("SM_SecurityCam")
    p.box((0.08, 0.04, 0.08), (0, 0.06, 0), "M_PlasticWhite")                    # wall plate
    p.box((0.03, 0.08, 0.03), (0, 0.0, 0), "M_PlasticWhite")                     # arm
    p.box((0.1, 0.22, 0.1), (0, -0.12, -0.02), "M_PlasticWhite", rot_x=-0.25)    # body, tilted down
    p.cyl(0.035, 0.03, (0, -0.235, -0.05), "M_PlasticBlack", segs=16, axis="Y")  # lens
    p.sphere(0.01, (0.035, -0.2, 0.02), "M_RedPaint", segs=8)                    # recording light
    return p.finish(0.004)


def brochure():
    """Flat printed booklet; the top face shows SlotMI=M_SignFace:<MI> (brochure / magazine)."""
    p = Prop("SM_Brochure")
    p.box((0.21, 0.3, 0.006), (0, 0, 0.003), "M_Paper")
    p.box((0.205, 0.295, 0.001), (0, 0, 0.0065), "M_SignFace")
    return p.finish(0.0, fit_uv={"top:M_SignFace": (-0.1025, 0.1025, 0.1475, -0.1475)})


def fruit_bowl():
    p = Prop("SM_FruitBowl")
    p.cyl(0.14, 0.07, (0, 0, 0.035), "M_Ceramic", segs=24, radius2=0.1)
    for i, (dx, dy, col) in enumerate(((0.04, 0.0, "M_BinderRed"), (-0.04, 0.03, "M_Yellow"), (0.0, -0.05, "M_LeafLight"),
                                       (-0.03, -0.02, "M_BinderRed"), (0.02, 0.05, "M_Yellow"))):
        p.sphere(0.04, (dx, dy, 0.09 + (i % 2) * 0.02), col, segs=12)
    return p.finish(0.0)


BUILDERS = [desk, office_chair, lambda: filing_cabinet(4), lambda: filing_cabinet(2), monitor, keyboard,
            mug, fire_extinguisher, directors_chair, water_cooler, wet_floor_sign, cubicle_panel,
            ream_of_paper, hospital_bed, mop_bucket, coffee_machine,
            # office supplies
            binder, lambda: binder("M_BinderMint", "SM_BinderMint"), lambda: binder("M_BinderRed", "SM_BinderRed"),
            stapler, tape_dispenser, pen_cup, desk_lamp, desk_phone, pc_tower, laptop, printer, trash_bin, paper_ball,
            wall_clock, pass_reader, exit_sign,
            # bright corporate design
            plant_tall, plant_desk, planter_box, lobby_sofa, coffee_table, logo_sign, meeting_chair,
            # kitchen
            kitchen_counter, kitchen_counter_short, kitchen_island, moving_box, fridge,
             lambda: filing_shell(4), lambda: filing_shell(2), file_drawer, microwave, kettle, vending_machine, cafe_table, bar_stool,
            # figures
            mannequin, mannequin_guard, mannequin_custodian, stalker,
            # wishlist batch
            reception_desk, fridge_body, fridge_door, door, door_frame, utility_shelf,
            coffee_bag, tea_box, cup_stack, snack_bag, snack_box, soda_can, water_bottle, milk_carton,
            lunch_box, fruit_bowl, sandwich, pizza_slice, salad_bowl, donut,
            toilet, sink, mirror, hand_dryer, stall_door, shoes, sign_plaque, security_cam, brochure]


def export_fbx(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=os.path.join(EXPORT, obj.name + ".fbx"), use_selection=True,
                             object_types={"MESH"}, use_mesh_modifiers=True, mesh_smooth_type="FACE",
                             apply_scale_options="FBX_SCALE_UNITS", add_leaf_bones=False, bake_anim=False)


def main():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    os.makedirs(EXPORT, exist_ok=True)
    objs = [b() for b in BUILDERS]
    # lay them out in a row for the .blend (export happens at the origin first)
    for o in objs:
        export_fbx(o)
    x = 0.0
    for o in objs:
        w = o.dimensions.x
        o.location.x = x + w / 2
        x += w + 0.35
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(HERE, "OfficeProps.blend"))
    print(f"[TAKE 4471] {len(objs)} props -> {EXPORT}")
    return objs


if __name__ == "__main__":
    main()
