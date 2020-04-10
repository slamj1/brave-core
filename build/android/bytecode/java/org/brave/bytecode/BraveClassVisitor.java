/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.brave.bytecode;

import static org.objectweb.asm.Opcodes.ACC_PRIVATE;
import static org.objectweb.asm.Opcodes.ACC_PUBLIC;
import static org.objectweb.asm.Opcodes.ASM5;

import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.tree.AnnotationNode;
import org.objectweb.asm.tree.ClassNode;
import org.objectweb.asm.tree.MethodNode;

class BraveClassVisitor extends ClassVisitor {

    private ClassVisitor mTarget;

    class Method {
        public int access;
        public String name;
        public String desc;
        public String signature;
        public String[] exceptions;

        public Method(final int access, final String name, String desc,
                      String signature, String[] exceptions) {
            this.access = access;
            this.name = name;
            this.desc = desc;
            this.signature = signature;
            this.exceptions = exceptions;
        }

        public void makePublic() {
            access &= ~ACC_PRIVATE;
            access |= ACC_PUBLIC;
        }
    }

    class BraveMethodVisitor extends MethodVisitor {

        private Method mMethod;

        public BraveMethodVisitor(Method method, MethodVisitor mv) {
            super(ASM5, mv);
            this.mMethod = method;
        }

        @Override
        public void visitMethodInsn(int opcode,
                                    java.lang.String owner,
                                    java.lang.String name,
                                    java.lang.String descriptor,
                                    boolean isInterface) {
            if (shouldMakePublicMethod(owner, name) &&
                    opcode == INVOKESPECIAL) {
                System.out.println("use invoke virtual for call to method " +
                        owner + "." + name);
                // use invoke virtual because other classes can override
                // the method now
                opcode = INVOKEVIRTUAL;
            }
            owner = maybeChangeOwner(owner, name);
            super.visitMethodInsn(opcode, owner, name, descriptor, isInterface);
        }
    }

    protected String mName = "";
    protected String mSuperName = "";

    private Map<String, String> mSuperNames = new HashMap<String, String>();
    private Map<String, ArrayList<String>> mDeleteMethods =
            new HashMap<String, ArrayList<String>>();
    private Map<String, ArrayList<String>> mMakePublicMethods =
            new HashMap<String, ArrayList<String>>();
    private Map<String, Map<String, String>> mChangeOwnerMethods =
            new HashMap<String, Map<String, String>>();
    private Map<String, ArrayList<String>> mMakeProtectedFields =
            new HashMap<String, ArrayList<String>>();
    private Map<String, Map<String, ArrayList<String>>> mAddAnnotations =
            new HashMap<String, Map<String, ArrayList<String>>>();

    public BraveClassVisitor(ClassVisitor visitor) {
        super(ASM5, null);
        mTarget = visitor;
    }

    protected void changeSuperName(String className, String superName) {
        mSuperNames.put(className, superName);
    }

    private boolean shouldDeleteMethod(String methodName) {
        for(Map.Entry<String, ArrayList<String>> entry :
                mDeleteMethods.entrySet()) {
            String className = entry.getKey();
            ArrayList<String> methodNames = entry.getValue();
            return mName.contains(className) &&
                   methodNames.contains(methodName);
        }

        return false;
    }

    protected void deleteMethod(String className, String methodName) {
        ArrayList methods = mDeleteMethods.get(className);
        if (methods == null) {
            methods = new ArrayList<String>();
            mDeleteMethods.put(className, methods);
        }
        methods.add(methodName);
    }

    private boolean shouldMakePublicMethod(String methodName) {
        for(Map.Entry<String, ArrayList<String>> entry :
                mMakePublicMethods.entrySet()) {
            String className = entry.getKey();
            ArrayList<String> methodNames = entry.getValue();
            return mName.contains(className) &&
                   methodNames.contains(methodName);
        }

        return false;
    }

    protected void makePublicMethod(String className, String methodName) {
        ArrayList methods = mMakePublicMethods.get(className);
        if (methods == null) {
            methods = new ArrayList<String>();
            mMakePublicMethods.put(className, methods);
        }
        methods.add(methodName);
    }

    private String maybeChangeOwner(String owner, String methodName) {
        if (mChangeOwnerMethods.containsKey(owner)) {
            Map<String, String> methods = mChangeOwnerMethods.get(owner);
            if (methods.containsKey(methodName)) {
                String newOwner = methods.get(methodName);
                if (!newOwner.equals(mName)) {
                    System.out.println("changing owner for " + mName + "." + methodName +
                            " - new owner " + newOwner);
                    return newOwner;
                }
            }
        }
        return owner;
    }

    protected void changeMethodOwner(String currentOwner, String methodName, String newOwner) {
        Map methods = mChangeOwnerMethods.get(currentOwner);
        if (methods == null) {
            methods = new HashMap<String, String>();
            mChangeOwnerMethods.put(currentOwner, methods);
        }
        methods.put(methodName, newOwner);
    }

    private boolean shouldDeleteField(String fieldName) {
        for(Map.Entry<String, ArrayList<String>> entry :
                mDeleteFields.entrySet()) {
            String className = entry.getKey();
            ArrayList<String> fieldNames = entry.getValue();
            return mName.contains(className) &&
                   fieldNames.contains(fieldName);
        }

        return false;
    }

    protected void deleteField(String className, String fieldName) {
        ArrayList fields = mDeleteFields.get(className);
        if (fields == null) {
            fields = new ArrayList<String>();
            mDeleteFields.put(className, fields);
        }
        fields.add(fieldName);
    }

    private boolean shouldMakeProtectedField(String className, String fieldName) {
        for(Map.Entry<String, ArrayList<String>> entry :
                mMakeProtectedFields.entrySet()) {
            String entryClassName = entry.getKey();
            ArrayList<String> fieldNames = entry.getValue();
            return className.contains(entryClassName) &&
                   fieldNames.contains(fieldName);
        }

        return false;
    }

    private boolean shouldMakeProtectedField(String fieldName) {
        return shouldMakeProtectedField(mName, fieldName);
    }

    protected void makeProtectedField(String className, String fieldName) {
        ArrayList fields = mMakeProtectedFields.get(className);
        if (fields == null) {
            fields = new ArrayList<String>();
            mMakeProtectedFields.put(className, fields);
        }
        fields.add(fieldName);
    }

    protected void addMethodAnnotation(String className, String methodName, String annotationType) {
        Map<String, ArrayList<String>> annotations = mAddAnnotations.get(className);
        if (annotations == null) {
            annotations = new HashMap<String, ArrayList<String>>();
            mAddAnnotations.put(className, annotations);
        }

        ArrayList<String> annotationList = annotations.get(methodName);
        if (annotationList == null) {
            annotationList = new ArrayList<String>();
            annotations.put(methodName, annotationList);
        }

        annotationList.add(annotationType);
    }

    @Override
    public void visit(int version,
                      int access,
                      String name,
                      String signature,
                      String superName,
                      String[] interfaces) {
        super.cv = new ClassNode();
        mName = name;
        if (mSuperNames.containsKey(name)) {
            superName = mSuperNames.get(name);
            System.out.println("change superclass of " + name + " to " + superName);
        }

        visitImpl(version, access, name, signature, superName, interfaces);
    }

    protected void visitImpl(int version,
                             int access,
                             String name,
                             String signature,
                             String superName,
                             String[] interfaces) {
        super.visit(version, access, name, signature, superName, interfaces);
    }

    @Override
    public MethodVisitor visitMethod(final int access,
                                     final String name,
                                     String desc,
                                     String signature,
                                     String[] exceptions) {
        Method method = new Method(access, name, desc, signature, exceptions);
        if (shouldDeleteMethod(name)) {
            System.out.println("delete " + name + " from " + mName);
            return null;
        }

        if (shouldMakePublicMethod(name)) {
            System.out.println("make " + name + " public in " + mName);
            method.makePublic();
        }

        return visitMethodImpl(method);
    }

    protected MethodVisitor visitMethodImpl(Method method) {
        return super.visitMethod(method.access,
                                 method.name,
                                 method.desc,
                                 method.signature,
                                 method.exceptions);
    }

    protected ClassNode process(ClassNode source) {
        Map<String, ArrayList<String>> annotationsForClass = mAddAnnotations.get(source.name);

        if (annotationsForClass == null)
            return source;

        List<MethodNode> methods = source.methods;
        for (MethodNode method : methods) {
            ArrayList<String> annotations = annotationsForClass.get(method.name);
            if (annotations != null) {
                if (method.visibleAnnotations == null) {
                    method.visibleAnnotations = new ArrayList<AnnotationNode>();
                }
                for (String annotation : annotations) {
                    method.visibleAnnotations.add(new AnnotationNode(annotation));
                    System.out.println("add " + annotation + " annotation to " + method.name + " in " + source.name);
                }
            }
        }
        return source;
    }

    @Override
    public void visitEnd() {
        super.visitEnd();
        ClassNode source = (ClassNode)super.cv;
        ClassNode result = process(source);
        if (mTarget != null) {
            result.accept(mTarget);
        }
    }
}
