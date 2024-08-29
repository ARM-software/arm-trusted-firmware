#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import sys
import re
from pydevicetree.ast import CellArray, LabelReference
from pydevicetree import Devicetree, Property, Node
from pathlib import Path
from typing import List, Optional

class COT:
    def __init__(self, inputfile: str, outputfile=None):
        try:
            self.tree = Devicetree.parseFile(inputfile)
        except:
            print("not a valid CoT DT file")
            exit(1)

        self.output = outputfile
        self.input = inputfile
        self.has_root = False

        # edge cases
        certs = self.get_all_certificates()
        for c in certs:
            if self.if_root(c):
                if not c.get_fields("signing-key"):
                    c.properties.append(Property("signing-key", CellArray([LabelReference("subject_pk")])))

    def print_cert_info(self, node:Node):
        img_id = node.get_field("image-id").values[0].replace('"', "")
        sign_key = self.get_sign_key(node)
        nv = self.get_nv_ctr(node)

        info = "<b>name:</b> {}<br><b>image-id:</b> {}<br>{}{}{}"\
                .format(node.name, img_id, "<b>root-certificate</b><br>" if self.if_root(node) else "", \
                     "<b>signing-key:</b> " + self.extract_label(sign_key) + "<br>" if sign_key else "", \
                     "<b>nv counter:</b> " + self.extract_label(nv) + "<br>" if nv else "")
        return info

    def print_data_info(self, node:Node):
        oid = node.get_field("oid")
        info = "<b>name:</b> {}<br><b>oid:</b> {}<br>" \
                .format(node.name, oid)

        return info

    def print_img_info(self, node:Node):
        hash = self.extract_label(node.get_fields("hash"))
        img_id = node.get_field("image-id").values[0].replace('"', "")
        info = "<b>name:</b> {}<br><b>image-id:</b> {}<br><b>hash:</b> {}"\
                .format(node.name, img_id, hash)

        return info

    def tree_width(self, parent_set, root):
        ans = 1
        stack = [root]

        while stack:
            tmp_stack = []
            while stack:
                cur_node = stack.pop()
                child = parent_set[cur_node]
                for c in child:
                    tmp_stack.append(c)

            stack = tmp_stack.copy()
            ans = max(ans, len(tmp_stack))

        return ans

    def resolve_lay(self, parent_set, lay, name_idx, root, bounds, break_name):
        child = parent_set[root]

        if len(child) == 0:
            return

        width = []
        total_width = 0
        for c in child:
            w = self.tree_width(parent_set, c)
            width.append(w)
            total_width += w

        allow_width = bounds[1] - bounds[0]
        interval = allow_width / total_width
        start = bounds[0]
        for i, c in enumerate(child):
            end = start + interval * width[i]
            new_bounds = [start, end]
            lay[name_idx[c]][0] = start + (end - start) / 2
            if end - start < 0.28:
                break_name.add(c)
            start = end
            self.resolve_lay(parent_set, lay, name_idx, c, new_bounds, break_name)

    def tree_visualization(self):
        import igraph
        from igraph import Graph, EdgeSeq
        import collections

        cert = self.get_certificates()
        pk = self.get_rot_keys()
        nv = self.get_nv_counters()
        image = self.get_images()

        certs = cert.children
        if pk:
            pks = pk.children
        else:
            pks = []
        nvs = nv.children
        images = image.children

        root_name = "CoT"

        G = Graph()
        detail = []
        lay = []
        name_idx = {}
        parent_set = collections.defaultdict(list)

        G.add_vertex(root_name)
        detail.append("CoT Root")
        name_idx[root_name] = len(lay)
        lay.append([0,0])

        G.add_vertex(cert.name)
        G.add_edge(root_name, cert.name)
        detail.append("All Certificates")
        name_idx[cert.name] = len(lay)
        lay.append([0, 1])
        parent_set[root_name].append(cert.name)

        if pk:
            G.add_vertex(pk.name)
            detail.append("All Public Trusted Key")
            G.add_edge(root_name, pk.name)
            name_idx[pk.name] = len(lay)
            lay.append([-2.0, 1])
            parent_set[root_name].append(pk.name)

        G.add_vertex(nv.name)
        detail.append("All NV Counters")
        G.add_edge(root_name, nv.name)
        name_idx[nv.name] = len(lay)
        lay.append([2.0, 1])
        parent_set[root_name].append(nv.name)

        if pks:
            for i, p in enumerate(pks):
                G.add_vertex(p.name)
                detail.append(self.print_data_info(p))
                G.add_edge(pk.name, p.name)
                name_idx[p.name] = len(lay)
                parent_set[pk.name].append(p.name)
                lay.append([0, lay[name_idx[pk.name]][1] + 1])

        for c in certs:
            G.add_vertex(c.name)
            detail.append(self.print_cert_info(c))
            name_idx[c.name] = len(lay)
            if self.if_root(c):
                G.add_edge(cert.name, c.name)
                parent_set[cert.name].append(c.name)
                lay.append([0, 2])
            else:
                parent = self.extract_label(c.get_fields("parent"))
                G.add_edge(parent, c.name)
                parent_set[parent].append(c.name)
                lay.append([0, lay[name_idx[parent]][1] + 1])

        for idx, i in enumerate(images):
            G.add_vertex(i.name)
            detail.append(self.print_img_info(i))
            parent = self.extract_label(i.get_fields("parent"))
            G.add_edge(parent, i.name)
            parent_set[parent].append(i.name)
            name_idx[i.name] = len(lay)
            lay.append([0, lay[name_idx[parent]][1] + 1])

        for i, n in enumerate(nvs):
            G.add_vertex(n.name)
            detail.append(self.print_data_info(n))
            G.add_edge(nv.name, n.name)
            name_idx[n.name] = len(lay)
            parent_set[nv.name].append(n.name)
            lay.append([0, lay[name_idx[nv.name]][1] + 1])

        break_name = set()
        self.resolve_lay(parent_set, lay, name_idx, root_name, [-3, 3], break_name)
        #lay = G.layout('rt')

        numVertex = len(G.get_vertex_dataframe())
        vertices = G.get_vertex_dataframe()
        v_label = []

        for i in vertices['name']:
            if i in break_name and len(i) > 10:
                middle = len(i) // 2
                v_label.append(i[:middle] + "<br>" + i[middle:])
            else:
                v_label.append(i)

        position = {k: lay[k] for k in range(numVertex)}
        Y = [lay[k][1] for k in range(numVertex)]
        M = max(Y)

        es = EdgeSeq(G) # sequence of edges
        E = [e.tuple for e in G.es] # list of edges

        L = len(position)
        Xn = [position[k][0] for k in range(L)]
        Yn = [2*M-position[k][1] for k in range(L)]
        Xe = []
        Ye = []
        for edge in E:
            Xe += [position[edge[0]][0], position[edge[1]][0], None]
            Ye += [2*M-position[edge[0]][1], 2*M-position[edge[1]][1], None]

        labels = v_label

        import plotly.graph_objects as go
        fig = go.Figure()
        fig.add_trace(go.Scatter(x = Xe,
                        y = Ye,
                        mode = 'lines',
                        line = dict(color='rgb(210,210,210)', width=2),
                        hoverinfo = 'none'
                        ))
        fig.add_trace(go.Scatter(x = Xn,
                        y = Yn,
                        mode = 'markers',
                        name = 'detail',
                        marker = dict(symbol = 'circle-dot',
                                        size = 50,
                                        color = 'rgba(135, 206, 250, 0.8)',    #'#DB4551',
                                        line = dict(color='MediumPurple', width=3)
                                        ),
                        text=detail,
                        hoverinfo='text',
                        hovertemplate =
                            '<b>Detail</b><br>'
                            '%{text}',
                        opacity=0.8
                        ))

        def make_annotations(pos, text, font_size=10, font_color='rgb(0,0,0)'):
            L = len(pos)
            if len(text) != L:
                raise ValueError('The lists pos and text must have the same len')
            annotations = []
            for k in range(L):
                annotations.append(
                    dict(
                        text = labels[k],
                        x = pos[k][0], y = 2*M-position[k][1],
                        xref = 'x1', yref = 'y1',
                        font = dict(color = font_color, size = font_size),
                        showarrow = False)
                )
            return annotations

        axis = dict(showline=False, # hide axis line, grid, ticklabels and  title
            zeroline=False,
            showgrid=False,
            showticklabels=False,
            )

        fig.update_layout(title= 'CoT Device Tree',
                    annotations=make_annotations(position, v_label),
                    font_size=12,
                    showlegend=False,
                    xaxis=axis,
                    yaxis=axis,
                    margin=dict(l=40, r=40, b=85, t=100),
                    hovermode='closest',
                    plot_bgcolor='rgb(248,248,248)'
                    )

        fig.show()

        return

    def if_root(self, node:Node) -> bool:
        for p in node.properties:
            if p.name == "root-certificate":
                return True
        return False

    def get_sign_key(self, node:Node):
        for p in node.properties:
            if p.name == "signing-key":
                return p.values

        return None

    def get_nv_ctr(self, node:Node):
        for nv in node.properties:
            if nv.name == "antirollback-counter":
                return nv.values

        return None

    def extract_label(self, label) -> str:
        if not label:
            return label
        return label[0].label.name

    def get_auth_data(self, node:Node):
        return node.children

    def format_auth_data_val(self, node:Node, cert:Node):
        type_desc = node.name
        ptr = type_desc + "_buf"
        len = "HASH_DER_LEN"
        if re.search("_pk$", type_desc):
            len = "PK_DER_LEN"

        # edge case
        if not self.if_root(cert) and "key_cert" in cert.name:
            if "content_pk" in ptr:
                ptr = "content_pk_buf"

        return type_desc, ptr, len

    def get_node(self, nodes: List[Node], name: str) -> Node:
        for i in nodes:
            if i.name == name:
                return i

    def get_certificates(self) -> Node:
        children = self.tree.children
        for i in children:
            if i.name == "cot":
                return self.get_node(i.children, "manifests")

    def get_images(self)-> Node:
        children = self.tree.children
        for i in children:
            if i.name == "cot":
                return self.get_node(i.children, "images")

    def get_nv_counters(self) -> Node:
        children = self.tree.children
        return self.get_node(children, "non_volatile_counters")

    def get_rot_keys(self) -> Node:
        children = self.tree.children
        return self.get_node(children, "rot_keys")

    def get_all_certificates(self) -> Node:
        cert = self.get_certificates()
        return cert.children

    def get_all_images(self) -> Node:
        image = self.get_images()
        return image.children

    def get_all_nv_counters(self) -> Node:
        nv = self.get_nv_counters()
        return nv.children

    def get_all_pks(self) -> Node:
        pk = self.get_rot_keys()
        if not pk:
            return []
        return pk.children

    def validate_cert(self, node:Node) -> bool:
        valid = True
        if not node.has_field("image-id"):
            print("{} missing mandatory attribute image-id".format(node.name))
            valid = False

        if not node.has_field("root-certificate"):
            if not node.has_field("parent"):
                print("{} missing mandatory attribute parent".format(node.name))
                valid = False
            else:
                # check if refer to non existing parent
                certs = self.get_all_certificates()
                found = False
                for c in certs:
                    if c.name == self.extract_label(node.get_fields("parent")):
                        found = True

                if not found:
                    print("{} refer to non existing parent".format(node.name))
                    valid = False

        else:
            self.has_root = True

        child = node.children
        if child:
            for c in child:
                if not c.has_field("oid"):
                    print("{} missing mandatory attribute oid".format(c.name))
                    valid = False

        return valid

    def validate_img(self, node:Node) -> bool:
        valid = True
        if not node.has_field("image-id"):
            print("{} missing mandatory attribute image-id".format(node.name))
            valid = False

        if not node.has_field("parent"):
            print("{} missing mandatory attribute parent".format(node.name))
            valid = False

        if not node.has_field("hash"):
            print("{} missing mandatory attribute hash".format(node.name))
            valid = False

        # check if refer to non existing parent
        certs = self.get_all_certificates()
        found = False
        for c in certs:
            if c.name == self.extract_label(node.get_fields("parent")):
                found = True

        if not found:
            print("{} refer to non existing parent".format(node.name))
            valid = False

        return valid

    def validate_nodes(self) -> bool:
        valid = True

        certs = self.get_all_certificates()
        images = self.get_all_images()

        for n in certs:
            node_valid = self.validate_cert(n)
            valid = valid and node_valid

        for i in images:
            node_valid = self.validate_img(i)
            valid = valid and node_valid

        if not self.has_root:
            print("missing root certificate")

        return valid

    def include_to_c(self, f):
        f.write("#include <stddef.h>\n")
        f.write("#include <mbedtls/version.h>\n")
        f.write("#include <common/tbbr/cot_def.h>\n")
        f.write("#include <drivers/auth/auth_mod.h>\n")
        f.write("#include <platform_def.h>\n\n")
        return

    def generate_header(self, output):
        self.include_to_c(output)

    def all_cert_to_c(self, f):
        certs = self.get_all_certificates()
        for c in certs:
            self.cert_to_c(c, f)

        f.write("\n")

    def cert_to_c(self, node: Node, f):
        node_image_id: int = node.get_field("image-id")

        f.write(f"static const auth_img_desc_t {node.name} = {{\n")
        f.write(f"\t.img_id = {node_image_id},\n")
        f.write("\t.img_type = IMG_CERT,\n")

        if not self.if_root(node):
            node_parent: Node = node.get_field("parent")

            f.write(f"\t.parent = &{node_parent.label.name},\n")
        else:
            f.write("\t.parent = NULL,\n")

        sign = self.get_sign_key(node)
        nv_ctr = self.get_nv_ctr(node)

        if sign or nv_ctr:
            f.write("\t.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {\n")

        if sign:
            f.write("\t\t[0] = {\n")
            f.write("\t\t\t.type = AUTH_METHOD_SIG,\n")
            f.write("\t\t\t.param.sig = {\n")

            f.write("\t\t\t\t.pk = &{},\n".format(self.extract_label(sign)))
            f.write("\t\t\t\t.sig = &sig,\n")
            f.write("\t\t\t\t.alg = &sig_alg,\n")
            f.write("\t\t\t\t.data = &raw_data\n")
            f.write("\t\t\t}\n")
            f.write("\t\t}}{}\n".format("," if nv_ctr else ""))

        if nv_ctr:
            f.write("\t\t[1] = {\n")
            f.write("\t\t\t.type = AUTH_METHOD_NV_CTR,\n")
            f.write("\t\t\t.param.nv_ctr = {\n")

            f.write("\t\t\t\t.cert_nv_ctr = &{},\n".format(self.extract_label(nv_ctr)))
            f.write("\t\t\t\t.plat_nv_ctr = &{}\n".format(self.extract_label(nv_ctr)))

            f.write("\t\t\t}\n")
            f.write("\t\t}\n")

        f.write("\t},\n")

        auth_data = self.get_auth_data(node)
        if auth_data:
            f.write("\t.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {\n")

            for i, d in enumerate(auth_data):
                type_desc, ptr, data_len = self.format_auth_data_val(d, node)

                f.write("\t\t[{}] = {{\n".format(i))
                f.write("\t\t\t.type_desc = &{},\n".format(type_desc))
                f.write("\t\t\t.data = {\n")

                f.write("\t\t\t\t.ptr = (void *){},\n".format(ptr))

                f.write("\t\t\t\t.len = (unsigned int){}\n".format(data_len))
                f.write("\t\t\t}\n")

                f.write("\t\t}}{}\n".format("," if i != len(auth_data) - 1 else ""))

            f.write("\t}\n")

        f.write("};\n\n")

        return


    def img_to_c(self, node:Node, f):
        node_image_id: int = node.get_field("image-id")
        node_parent: Node = node.get_field("parent")
        node_hash: Node = node.get_field("hash")

        f.write(f"static const auth_img_desc_t {node.name} = {{\n")
        f.write(f"\t.img_id = {node_image_id},\n")
        f.write("\t.img_type = IMG_RAW,\n")
        f.write(f"\t.parent = &{node_parent.label.name},\n")
        f.write("\t.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {\n")

        f.write("\t\t[0] = {\n")
        f.write("\t\t\t.type = AUTH_METHOD_HASH,\n")
        f.write("\t\t\t.param.hash = {\n")
        f.write("\t\t\t\t.data = &raw_data,\n")
        f.write(f"\t\t\t\t.hash = &{node_hash.label.name}\n")
        f.write("\t\t\t}\n")

        f.write("\t\t}\n")
        f.write("\t}\n")
        f.write("};\n\n")

        return

    def all_img_to_c(self, f):
        images = self.get_all_images()
        for i in images:
            self.img_to_c(i, f)

        f.write("\n")

    def nv_to_c(self, f):
        nv_ctr = self.get_all_nv_counters()

        for nv in nv_ctr:
            nv_oid: str = nv.get_field("oid")

            f.write(f"static auth_param_type_desc_t {nv.name} = "\
                    f"AUTH_PARAM_TYPE_DESC(AUTH_PARAM_NV_CTR, \"{nv_oid}\");\n")

        f.write("\n")

        return

    def pk_to_c(self, f):
        pks = self.get_all_pks()

        for p in pks:
            pk_oid: str = p.get_field("oid")

            f.write(f"static auth_param_type_desc_t {p.name} = "\
                    f"AUTH_PARAM_TYPE_DESC(AUTH_PARAM_PUB_KEY, \"{pk_oid}\");\n")

        f.write("\n")
        return

    def buf_to_c(self, f):
        certs = self.get_all_certificates()

        buffers = set()

        for c in certs:
            auth_data = self.get_auth_data(c)

            for a in auth_data:
                type_desc, ptr, data_len = self.format_auth_data_val(a, c)

                if not ptr in buffers:
                    f.write(f"static unsigned char {ptr}[{data_len}];\n")
                    buffers.add(ptr)

        f.write("\n")

    def param_to_c(self, f):
        f.write("static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_PUB_KEY, 0);\n")
        f.write("static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_SIG, 0);\n")
        f.write("static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_SIG_ALG, 0);\n")
        f.write("static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_RAW_DATA, 0);\n")
        f.write("\n")

        certs = self.get_all_certificates()
        for c in certs:
            hash = c.children
            for h in hash:
                name = h.name
                oid = h.get_field("oid")

                if re.search("_pk$", name):
                    ty = "AUTH_PARAM_PUB_KEY"
                elif re.search("_hash$", name):
                    ty = "AUTH_PARAM_HASH"

                f.write(f"static auth_param_type_desc_t {name} = "\
                    f"AUTH_PARAM_TYPE_DESC({ty}, \"{oid}\");\n")

        f.write("\n")

    def cot_to_c(self, f):
        certs = self.get_all_certificates()
        images = self.get_all_images()

        f.write("static const auth_img_desc_t * const cot_desc[] = {\n")

        for i, c in enumerate(certs):
            c_image_id: int = c.get_field("image-id")

            f.write(f"\t[{c_image_id}]	=	&{c.name},\n")

        for i, c in enumerate(images):
            c_image_id: int = c.get_field("image-id")

            f.write(f"\t[{c_image_id}]	=	&{c.name},\n")

        f.write("};\n\n")
        f.write("REGISTER_COT(cot_desc);\n")
        return

    def generate_c_file(self):
        filename = Path(self.output)
        filename.parent.mkdir(exist_ok=True, parents=True)

        with open(self.output, 'w+') as output:
            self.generate_header(output)
            self.buf_to_c(output)
            self.param_to_c(output)
            self.nv_to_c(output)
            self.pk_to_c(output)
            self.all_cert_to_c(output)
            self.all_img_to_c(output)
            self.cot_to_c(output)

        return
