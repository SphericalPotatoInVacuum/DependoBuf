#![allow(unused_variables, unused_imports, unused_parens, dead_code)]
//! This file was automatically generated by DependoBuf

pub enum FieldsDependOnDependencies {
    FieldsDependOnDependencies{
        v: std::rc::Rc<Vec>,
    },
}
use FieldsDependOnDependencies::*;

impl FieldsDependOnDependencies {
    pub fn check(&self, n: &std::rc::Rc<Nat>, b: bool) -> bool {
        match self {
            FieldsDependOnDependencies { v } => {
                if !v.check() { return false; }
            }
        }
        return true;
    }
}

