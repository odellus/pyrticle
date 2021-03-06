from __future__ import division




def generate_arc_points(radius, start_angle, end_angle, 
        subdiv_degrees, include_final=True):
    from math import ceil, sin, cos, pi
    point_count = int(ceil((end_angle-start_angle)/subdiv_degrees))
    dphi = (end_angle-start_angle)/point_count

    if include_final:
        end_step = point_count + 1
    else:
        end_step = point_count

    for i in range(end_step):
        phi = pi/180*(start_angle + i*dphi)
        yield cos(phi)*radius, sin(phi)*radius

def make_magnetron_outline(
        cavity_count,
        cavity_angle,
        radius_cathode,
        radius_anode,
        radius_outer,
        cathode_marker,
        anode_marker,
        subdiv_degrees,
        anode_cavities={},
        ):

    from math import sin, cos

    def round_trip_connect(start, end):
        for i in range(start, end):
            yield i, i+1
        yield end, start

    anode_points = []
    cathode_points = []
    anode_facet_markers = []

    angle_step = 360/cavity_count

    for cav_idx in range(cavity_count):
        start_angle = angle_step * cav_idx
        if cav_idx in anode_cavities:
            anode_cavities[cav_idx](
                    start_angle,
                    angle_step,
                    anode_points,
                    anode_facet_markers)
        else:
            anode_point_count_before = len(anode_points)
            anode_points.extend(generate_arc_points(
                radius_outer, start_angle, start_angle+cavity_angle,
                subdiv_degrees))
            anode_points.extend(generate_arc_points(
                radius_anode, 
                start_angle+cavity_angle, 
                start_angle+angle_step,
                subdiv_degrees))
            anode_facet_markers.extend([anode_marker] * 
                    (len(anode_points) - anode_point_count_before))


    cathode_points = list(
            generate_arc_points(radius_cathode, 0, 360, subdiv_degrees,
                include_final=False))

    return (cathode_points+anode_points, 
            list(round_trip_connect(0, len(cathode_points)-1))
            + list(round_trip_connect(
                len(cathode_points), 
                len(cathode_points)+len(anode_points)-1)),
            len(cathode_points)*[cathode_marker] + anode_facet_markers)




def make_poly_mesh(points, facets, facet_markers, refinement_func=None):
    import meshpy.triangle as triangle
    mesh_info = triangle.MeshInfo()
    mesh_info.set_points(points)
    mesh_info.set_facets(facets, facet_markers)
    mesh_info.holes.resize(1)
    mesh_info.holes[0] = [0,0]
    return triangle.build(mesh_info, 
            refinement_func=refinement_func,
            generate_edges=True)




class A6Triangulator:
    cavity_angle = 20
    radius_cathode = 0.0158
    radius_anode = 0.0211
    radius_outer = 0.0411

    cathode_marker = 1
    anode_marker = 2
    open_marker = 3

    horn_radius = 0.07

    subdiv_degrees = 5

    def make_outline(self):
        def make_horn(start_angle, angle_step,
                anode_points, anode_facet_markers):
            from math import sin, pi
            anode_points.extend([
                (self.horn_radius,0),
                (self.horn_radius,sin(pi/180*self.cavity_angle)
                    *self.horn_radius),
                ])
            anode_facet_markers.extend([
                self.open_marker, 
                self.anode_marker])
            anode_point_count_before = len(anode_points)
            anode_points.extend(generate_arc_points(
                self.radius_anode, 
                start_angle+self.cavity_angle, 
                start_angle+angle_step,
                self.subdiv_degrees))
            anode_facet_markers.extend([self.anode_marker] * 
                    (len(anode_points) - anode_point_count_before))

        return make_magnetron_outline(
            cavity_count=6,
            cavity_angle=self.cavity_angle,
            radius_cathode=self.radius_cathode,
            radius_anode=self.radius_anode,
            radius_outer=self.radius_outer,
            anode_marker=self.anode_marker,
            cathode_marker=self.cathode_marker,
            anode_cavities={0:make_horn},
            subdiv_degrees=self.subdiv_degrees,
            )

    def make_triangulation(self, max_area):
        def refinement_func(tri, area):
            return area > max_area

        points, facets, facet_markers = self.make_outline()
        return make_poly_mesh(
                points,
                facets,
                facet_markers,
                refinement_func
                )




def main():
    a6 = A6Triangulator()
    mesh = a6.make_triangulation(max_area=2e-6)

    import meshpy.triangle as triangle
    triangle.write_gnuplot_mesh("magnetron.dat", mesh)
    mesh.write_neu(open("magnetron.neu", "w"),
            bc={
                a6.cathode_marker:("cathode", a6.cathode_marker), 
                a6.anode_marker:("anode", a6.anode_marker),
                a6.open_marker:("open", a6.open_marker)})




if __name__ == "__main__":
    main()
