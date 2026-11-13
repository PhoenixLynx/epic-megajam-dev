"""
make_office_props.py: TAKE 4471 office props, built procedurally in Blender (5.2).

Run inside Blender: Scripting tab > Open > this file > Run Script
   or headless:      blender -b -P make_office_props.py

Builds 16 low-poly props, each with its pivot at the floor (bottom centre),
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

    def finish(self, bevel=0.004):
        mesh = bpy.data.meshes.new(self.name)
        bmesh.ops.remove_doubles(self.bm, verts=self.bm.verts, dist=0.0001)
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


def monitor():
    p = Prop("SM_Monitor")
    p.box((0.22, 0.18, 0.015), (0, 0.02, 0.0075), "M_PlasticBlack")
    p.box((0.05, 0.03, 0.28), (0, 0.07, 0.15), "M_PlasticBlack")
    p.box((0.62, 0.035, 0.38), (0, 0.04, 0.38), "M_PlasticBlack")
    p.box((0.59, 0.004, 0.335), (0, 0.021, 0.39), "M_Screen")                 # screen face: own slot for UI later
    return p.finish(0.003)


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
    for y, tilt in ((-0.12, 0.2), (0.12, -0.2)):
        p.box((0.3, 0.015, 0.62), (0, y, 0.3), "M_Yellow", rot_x=tilt)
    p.box((0.06, 0.06, 0.03), (0, 0, 0.62), "M_Yellow")
    return p.finish(0.006)


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


BUILDERS = [desk, office_chair, lambda: filing_cabinet(4), lambda: filing_cabinet(2), monitor, keyboard,
            mug, fire_extinguisher, directors_chair, water_cooler, wet_floor_sign, cubicle_panel,
            ream_of_paper, hospital_bed, mop_bucket, coffee_machine]


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
