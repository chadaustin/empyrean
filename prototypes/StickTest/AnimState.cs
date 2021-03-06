using System;
using System.Collections;
using StickTest.MilkShape;

namespace StickTest
{
	/// <summary>
	/// Summary description for AnimState.
	/// </summary>
	public class AnimState
	{
        /// <summary>
        /// Isolating one component of the interpolation that goes on.  Head hurting from trying to do both.
        /// </summary>
        public class KeyInterpolator
        {
            double time;
            int nextkey;

            Key[] keys;

            Vector cur;
            Vector _base;       // base is a keyword

            Vector delta;       // amount of change that needs to happen between the last key, and the next

            public KeyInterpolator(Key[] k,Vector basevec)
            {
                keys=k;
            
                cur=new Vector(0,0,0);
                _base=new Vector(basevec);

                NextKey=0;
            }

            /// <summary>
            /// Animates, yo.
            /// </summary>
            /// <param name="td">time delta; the amount of animating to do</param>
            public void Animate(double td)
            {
                switch (1)
                {
                    case 1:
                        if (time+td>keys[nextkey].time)
                        {
                            if (keys.Length==1) return;

                            double _t=keys[nextkey].time-time;

                            cur+=delta*_t;                  // get the rest of this keyframe added in

                            td-=keys[nextkey].time-time;    // adjust time delta to add other keyframes
                            time=keys[nextkey].time;

                            NextKey++;
                            goto case 1;    // GOTO lives on.
                        }
                        break;
                }

                cur+=delta*td;
                time+=td;
            }

            int NextKey
            {
                get {   return nextkey; }
                set
                {
                    nextkey=value;
                    if (nextkey>=keys.Length)
                    {
                        time=1;
                        nextkey=0;
                    }

                    if (time==keys[nextkey].time)
                    {
                        delta=keys[nextkey].v;
                    }
                    else
                        delta=(keys[nextkey].v-cur)*(1/(keys[nextkey].time-time));
                }
            }

            public Vector Current   {   get {   return cur;     }   }
            public Vector Base      {   get {   return _base;   }   } 

            public override string ToString()
            {
                return String.Format("{0},{1},{2}\n   {3},{4},{5}\n   {6}",cur.x,cur.y,cur.z,delta.x,delta.y,delta.z,time);
            }
        }

        public class JointState
        {
            public KeyInterpolator pos,rot;
            public string name;
            public ArrayList children=new ArrayList();

            public JointState(Joint j)
            {
                pos=new KeyInterpolator(j.PosKeys,j.initpos);
                rot=new KeyInterpolator(j.RotKeys,j.initrot);
                name=j.name;
            }

            public void Animate(double td)
            {
                pos.Animate(td);
                rot.Animate(td);
            }
        }

        // The triangle list stuff really doesn't belong here.  But it doesn't really belong anywhere at the moment.
        public struct TriangleList
        {
            public int[] vertices;
            public int[] normals;
        }

        public class MeshData
        {
            public Vector[] transformedvertices;
            public Vector[] untransformedvertices;
            public Vector[] transformednormals;
            public Vector[] untransformednormals;
            public TriangleList[] trianglelists;
        }

        Model model;
        JointState[] joints;
        JointState rootjoint;

        /*Vector[][] transformedvertices;
        Vector[][] untransformedvertices;
        int[][][] trianglelists; // @_@*/
        MeshData[] meshdata;

        JointState FindJointByName(string name)
        {
            foreach (JointState js in joints)
                if (js.name==name)
                    return js;
            return null;
        }

        public AnimState(Model m)
		{
            model=m;
            joints=new JointState[m.joints.Length];
            for (int i=0; i<joints.Length; i++)
                joints[i]=new JointState(m.joints[i]);

            /*
             * Reconstruct the bone heirarchy.
             * This isn't technically "correct", as it makes assumptions about the order which milkshape writes the bones.  There's
             * no reason why a bone's parent couldn't appear after it in this list.
             */
            for (int i=0; i<joints.Length; i++)
            {
                JointState j=FindJointByName(m.joints[i].parentname);
                if (j==null)
                    rootjoint=joints[i];            // if we can't find the parent, we assume that this is the root bone (probably not the Best Thing, but it works for now)
                else
                    j.children.Add(joints[i]);
            }

            /*
             * Initialize the current vertex arrangement.  Basically a copy operation.
             */
            meshdata=new MeshData[m.meshes.Length];

            for (int i=0; i<m.meshes.Length; i++)
            {
                meshdata[i]=new MeshData();

                Vertex[] verts=m.meshes[i].vertices;
                Vector[] v=new Vector[verts.Length];
                for (int j=0; j<v.Length; j++)
                    v[j]=new Vector(verts[j].x,verts[j].y,verts[j].z);

                meshdata[i].transformedvertices=v;
                meshdata[i].untransformedvertices=new Vector[verts.Length];  // filled up in UndeformJoint
            }

            if (rootjoint!=null)
                UndeformJoint(rootjoint,Matrix.identity);

            for (int i=0; i<model.meshes.Length; i++)
                GenerateTriangleLists(i);
		}

        void UndeformJoint(JointState j, Matrix parentm)
        {
            Matrix invTrans = Matrix.TranslationMatrix(-j.pos.Base.x,-j.pos.Base.y,-j.pos.Base.z);
            Matrix invRot   = Matrix.RotationMatrix   ( j.rot.Base.x, j.rot.Base.y, j.rot.Base.z);
            invRot.Transpose();
            Matrix m = invRot * invTrans * parentm;

            for (int i=0; i<model.meshes.Length; i++)
            {
                Mesh mesh=model.meshes[i];

                for (int k=0; k<mesh.vertices.Length; k++)
                {
                    Vertex v=mesh.vertices[k];
                    if (joints[v.jointidx]==j)
                    {
                        meshdata[i].untransformedvertices[k] = m*new Vector(v.x,v.y,v.z);
                    }
                }
            }

            foreach (JointState k in j.children)
            {
                UndeformJoint(k,m);
            }
        }

        void DeformJoint(JointState j,Matrix parentm)
        {
            if (j==null)
                return;

            Matrix m=
                parentm *
                Matrix.TranslationMatrix(j.pos.Base.x,j.pos.Base.y,j.pos.Base.z) *
                Matrix.RotationMatrix(j.rot.Base.x,j.rot.Base.y,j.rot.Base.z) *
                Matrix.TranslationMatrix(j.pos.Current.x, j.pos.Current.y, j.pos.Current.z) *
                Matrix.RotationMatrix(j.rot.Current.x, j.rot.Current.y, j.rot.Current.z);


            for (int i=0; i<model.meshes.Length; i++)
            {
                Mesh mesh=model.meshes[i];

                for (int k=0; k<mesh.vertices.Length; k++)
                {
                    Vector v=meshdata[i].untransformedvertices[k];

                    if (joints[mesh.vertices[k].jointidx]==j)
                    {
                        meshdata[i].transformedvertices[k]=m*new Vector(v.x,v.y,v.z);
                    }
                }
            }

            foreach (JointState k in j.children)
            {
                DeformJoint(k,m);
            }
        }

        public void Animate(double dt)
        {
            foreach (JointState js in joints)
                js.Animate(dt);

            DeformJoint(rootjoint,Matrix.identity);
        }
        public Vector[] GetVerts(int i)
        {
            return meshdata[i].transformedvertices;
        }
        void GenerateTriangleLists(int meshidx)
        {
            ArrayList list=new ArrayList(); // list of triangle strips (int[]s)
            ArrayList tris=new ArrayList(); // lists of arrays of vertex indeces of as-yet un-listified triangles (one triangle == 3 indeces)
            Mesh mesh=model.meshes[meshidx];

            // first, get a list of all the triangles
            for (int i=0; i<mesh.triangles.Length; i++)
            {
                tris.Add(mesh.triangles[i]);
            }

            // now, pick a triangle, and find triangles still in the list, that share two vertices.
            // If we find one, add the third vertex to the list, and continue on.  Else add what we've got to list, and start anew.
            ArrayList curv=new ArrayList(); // list of ints
            ArrayList curn=new ArrayList();
            while (tris.Count>0)
            {
                curv.Clear();   curn.Clear();
                Mesh.Triangle t=(Mesh.Triangle)tris[0];
                curv.Add(t.i[0]);   curv.Add(t.i[1]);   curv.Add(t.i[2]);
                curn.Add(t.n[0]);   curn.Add(t.n[1]);   curn.Add(t.n[2]);

                tris.RemoveAt(0);
                TriangleList tlist=GenList(curv,curn,tris,meshidx);
                list.Add(tlist);
            }

            meshdata[meshidx].trianglelists=(TriangleList[])list.ToArray(typeof(TriangleList));

            // just for fun -- find out how many lists we have, and how long they are
            
        }

        // recursive thinger for grabbing a wad of triangles and arranging them into a list
        TriangleList GenList(ArrayList verts,ArrayList normals,ArrayList tris,int meshidx)
        {
            // get the last two points on the list
            int[] lastpoints={(int)verts[verts.Count-2],(int)verts[verts.Count-1]};

            foreach (Mesh.Triangle tri in tris)
            {
                int shared=0,unsharedidx=-1;
                for (int i=0; i<3; i++)
                    if (tri.i[i]==lastpoints[0] || tri.i[i]==lastpoints[1])
                        shared++;
                    else
                        unsharedidx=i;

                if (shared==2)
                {
                    verts.Add(tri.i[unsharedidx]);
                    normals.Add(tri.n[unsharedidx]);
                    
                    tris.Remove(tri);
                    return GenList(verts,normals,tris,meshidx);
                }
            }

            // can't find any more. :(
            TriangleList list=new TriangleList();
            list.vertices=(int[])verts.ToArray(typeof(int));
            list.normals=(int[])normals.ToArray(typeof(int));
            return list;
        }

        public TriangleList[] GetTriangleLists(int modelidx)
        {
            return meshdata[modelidx].trianglelists;
        }
    }
}
