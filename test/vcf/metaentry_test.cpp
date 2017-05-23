/**
 * Copyright 2015 EMBL - European Bioinformatics Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include <iostream>
#include <typeinfo>

#include "catch/catch.hpp"

#include "vcf/file_structure.hpp"
#include "vcf/error.hpp"

namespace ebi
{

    TEST_CASE("MetaEntry constructor (no value)", "[constructor][novalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v41,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
            SECTION ("It should work with any ID and source")
            {
                CHECK_NOTHROW( (vcf::MetaEntry { 1, "reference", std::make_shared<vcf::Source>(source) }) );
            }
            
            SECTION ("No value should be assigned")
            {
                auto meta = vcf::MetaEntry { 1, "reference", std::make_shared<vcf::Source>(source) } ;
                
                CHECK( meta.id == "reference" );
                CHECK( meta.structure == vcf::MetaEntry::Structure::NoValue );
                CHECK( boost::get<std::string>(meta.value) == std::string {} );
                CHECK_THROWS_AS( (boost::get<std::map<std::string,std::string>>(meta.value)),
                                boost::bad_get);
            }
    }

    TEST_CASE("MetaEntry constructor (plain value)", "[constructor][plainvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v42,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};

            
            SECTION("Correct arguments")
            {
                CHECK_NOTHROW( (vcf::MetaEntry { 1, "assembly", "GRCh37", std::make_shared<vcf::Source>(source) }) );
            }
            
            SECTION("A one-line string value should be assigned")
            {
                auto meta = vcf::MetaEntry { 1, "assembly", "GRCh37", std::make_shared<vcf::Source>(source) } ;
                        
                CHECK( meta.structure == vcf::MetaEntry::Structure::PlainValue );
                CHECK( meta.id == "assembly" );
                CHECK( boost::get<std::string>(meta.value) == std::string{"GRCh37"} );
                CHECK_THROWS_AS( (boost::get<std::map<std::string,std::string>>(meta.value)),
                                boost::bad_get);
            }
                
            SECTION("A multi-line string value should throw an error")
            {
                CHECK_THROWS_AS( (vcf::MetaEntry { 1, "assembly", "GRCh37\nGRCh37", std::make_shared<vcf::Source>(source) } ),
                                vcf::MetaSectionError* );
            }
    }

    TEST_CASE("MetaEntry constructor (key-value pairs)", "[constructor][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v43,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("Correct arguments")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                            1,
                            "contig",
                            { {"ID", "contig_1"} },
                            std::make_shared<vcf::Source>(source) } ) );
        }
            
        
        SECTION("A key-pair map should be assigned")
        {
            auto meta = vcf::MetaEntry {
                            1,
                            "contig",
                            { {"ID", "contig_1"} },
                            std::make_shared<vcf::Source>(source) } ;

            CHECK( meta.id == "contig" );
            CHECK( meta.structure == vcf::MetaEntry::Structure::KeyValue );
            CHECK_THROWS_AS( boost::get<std::string>(meta.value),
                            boost::bad_get);
            CHECK( (boost::get<std::map<std::string,std::string>>(meta.value)) == (std::map<std::string,std::string>{ {"ID", "contig_1"} }) );
        }
        
    }
    
    
    TEST_CASE("ALT MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v41,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID and Description presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "INS"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "TAG_ID"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
        
        SECTION("ID prefixes")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "DEL"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                              
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "INS"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                             
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "DUP"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                             
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "INV"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                             
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "CNV"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                               
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "DEL:FOO"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "INS:FOO"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "DUP:FOO"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "INV:FOO"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "CNV:FOO"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                  
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "ALT",
                                { {"ID", "CNV:FOO:BAR"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
        }
    }

    TEST_CASE("contig MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v42,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "contig",
                                { {"ID", "contig_1"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry { 
                                1,
                                "contig",
                                { {"ID", "contig_2"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "contig",
                                { {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
    }
    
    TEST_CASE("FILTER MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v43,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID and Description presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FILTER",
                                { {"ID", "Filter1"}, {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FILTER",
                                { {"Description", "tag_description"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FILTER",
                                { {"ID", "TAG_ID"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
    }
    
    TEST_CASE("FORMAT MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v41,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID, Number, Type and Description presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"Number", "1"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "1"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "String"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
        
        SECTION("Number field values")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "10"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "A"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "G"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "."}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "10a"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "D"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
        
        SECTION("Type field values")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "10"}, {"Type", "Integer"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "A"}, {"Type", "Float"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "R"}, {"Type", "Character"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "G"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "."}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "FORMAT",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "int"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
    }
    
    TEST_CASE("INFO MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v43,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID, Number, Type and Description presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"Number", "1"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "1"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "String"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
        
        SECTION("Number field values")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "10"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "A"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "G"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "."}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "10a"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "D"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
        
        SECTION("Type field values")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "10"}, {"Type", "Integer"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "A"}, {"Type", "Float"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                     
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "A"}, {"Type", "Flag"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                               
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "R"}, {"Type", "Character"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "G"}, {"Type", "String"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                            
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "."}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "GT"}, {"Number", "1"}, {"Type", "int"}, {"Description", "Genotype"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }

        SECTION("INFO predefined tags") {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AA"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Ancestral Allele"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AA"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Ancestral Allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AA"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Ancestral Allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AC"}, {"Number", "A"}, {"Type", "Integer"}, {"Description", "Allele count in genotypes, for each ALT allele, in the same order as listed"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AC"}, {"Number", "A"}, {"Type", "Float"}, {"Description", "Allele count in genotypes, for each ALT allele, in the same order as listed"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AC"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Allele count in genotypes, for each ALT allele, in the same order as listed"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AD"}, {"Number", "R"}, {"Type", "Integer"}, {"Description", "Total read depth for each allele"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AD"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Total read depth for each allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AD"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Total read depth for each allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADF"}, {"Number", "R"}, {"Type", "Integer"}, {"Description", "Read depth for each allele on the forward strand"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADF"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Read depth for each allele on the forward strand"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADF"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Read depth for each allele on the forward strand"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
  
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADR"}, {"Number", "R"}, {"Type", "Integer"}, {"Description", "Read depth for each allele on the reverse strand"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADR"}, {"Number", "R"}, {"Type", "String"}, {"Description", "Read depth for each allele on the reverse strand"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "ADR"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Read depth for each allele on the reverse strand"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AF"}, {"Number", "A"}, {"Type", "Float"}, {"Description", "Allele frequency for each ALT allele in the same order as listed (estimated from primary data, not called genotypes)"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AF"}, {"Number", "A"}, {"Type", "Flag"}, {"Description", "Allele frequency for each ALT allele in the same order as listed (estimated from primary data, not called genotypes)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AF"}, {"Number", "1"}, {"Type", "Float"}, {"Description", "Allele frequency for each ALT allele in the same order as listed (estimated from primary data, not called genotypes)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AN"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Total number of alleles in called genotypes"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AN"}, {"Number", "1"}, {"Type", "Float"}, {"Description", "Total number of alleles in called genotypes"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "AN"}, {"Number", "A"}, {"Type", "Integer"}, {"Description", "Total number of alleles in called genotypes"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "BQ"}, {"Number", "1"}, {"Type", "Float"}, {"Description", "RMS base quality"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "BQ"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "RMS base quality"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "BQ"}, {"Number", "A"}, {"Type", "Float"}, {"Description", "RMS base quality"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "CIGAR"}, {"Number", "A"}, {"Type", "String"}, {"Description", "Cigar string describing how to align an alternate allele to the reference allele"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "CIGAR"}, {"Number", "A"}, {"Type", "Flag"}, {"Description", "Cigar string describing how to align an alternate allele to the reference allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "CIGAR"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Cigar string describing how to align an alternate allele to the reference allele"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DB"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "dbSNP membership"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DB"}, {"Number", "0"}, {"Type", "Float"}, {"Description", "dbSNP membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DB"}, {"Number", "A"}, {"Type", "Flag"}, {"Description", "dbSNP membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DP"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Combined depth across samples"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DP"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "Combined depth across samples"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "DP"}, {"Number", "A"}, {"Type", "Integer"}, {"Description", "Combined depth across samples"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "END"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "End position (for use with symbolic alleles)"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "END"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "End position (for use with symbolic alleles)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "END"}, {"Number", "A"}, {"Type", "Integer"}, {"Description", "End position (for use with symbolic alleles)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H2"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "HapMap2 membership"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H2"}, {"Number", "0"}, {"Type", "String"}, {"Description", "HapMap2 membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H2"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "HapMap2 membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H3"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "HapMap3 membership"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H3"}, {"Number", "0"}, {"Type", "String"}, {"Description", "HapMap3 membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "H3"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "HapMap3 membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "MQ0"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Number of MAPQ == 0 reads"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "MQ0"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Number of MAPQ == 0 reads"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "MQ0"}, {"Number", "0"}, {"Type", "Integer"}, {"Description", "Number of MAPQ == 0 reads"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "NS"}, {"Number", "1"}, {"Type", "Integer"}, {"Description", "Number of samples with data"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "NS"}, {"Number", "1"}, {"Type", "String"}, {"Description", "Number of samples with data"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "NS"}, {"Number", "0"}, {"Type", "Integer"}, {"Description", "Number of samples with data"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "SOMATIC"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "Somatic mutation (for cancer genomics)"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "SOMATIC"}, {"Number", "0"}, {"Type", "String"}, {"Description", "Somatic mutation (for cancer genomics)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "SOMATIC"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "Somatic mutation (for cancer genomics)"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "VALIDATED"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "Validated by follow-up experiment"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "VALIDATED"}, {"Number", "0"}, {"Type", "String"}, {"Description", "Validated by follow-up experiment"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "VALIDATED"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "Validated by follow-up experiment"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );

            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "1000G"}, {"Number", "0"}, {"Type", "Flag"}, {"Description", "1000 Genomes membership"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );

            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "1000G"}, {"Number", "0"}, {"Type", "String"}, {"Description", "1000 Genomes membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
 
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "INFO",
                                { {"ID", "1000G"}, {"Number", "1"}, {"Type", "Flag"}, {"Description", "1000 Genomes membership"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
    }
    
    TEST_CASE("SAMPLE MetaEntry checks", "[checks][keyvalue]") 
    {
        auto source = vcf::Source {
            "Example VCF source",
            vcf::InputFormat::VCF_FILE_VCF | vcf::InputFormat::VCF_FILE_BGZIP,
            vcf::Version::v43,
            vcf::Ploidy{2},
            {},
            { "Sample1", "Sample2", "Sample3" }};
            
        SECTION("ID presence")
        {
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "SAMPLE",
                                { {"ID", "Sample_1"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_NOTHROW( (vcf::MetaEntry {
                                1,
                                "SAMPLE",
                                { {"ID", "Sample_2"}, {"Genomes", "genome_1,genome_2"}, {"Mixtures", "mixture_1"} },
                                std::make_shared<vcf::Source>(source)
                            } ) );
                                
            CHECK_THROWS_AS( (vcf::MetaEntry {
                                1,
                                "SAMPLE",
                                { {"Genomes", "genome_1,genome_2"} },
                                std::make_shared<vcf::Source>(source)
                            }),
                            vcf::MetaSectionError* );
        }
    }
}
