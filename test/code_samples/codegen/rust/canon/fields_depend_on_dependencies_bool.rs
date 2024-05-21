#![allow(unused_variables, unused_imports, unused_parens, dead_code)]
//! This file was automatically generated by DependoBuf

pub struct FieldsDependOnDependenciesBool {
    pub v7: std::rc::Rc<Field>,
    pub v8: std::rc::Rc<Field>,
    pub v9: std::rc::Rc<Field>,
}

impl FieldsDependOnDependenciesBool {
    pub fn check(&self, b1: bool, b2: bool) -> bool {
        let v7 = &self.v7;
        let v8 = &self.v8;
        let v9 = &self.v9;
        if !v7.check() { return false; }
        if !v8.check() { return false; }
        if !v9.check() { return false; }
        return true;
    }
}

