/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
@Library(["cortex_jenkins_pipeline_lib", "cortex_jenkins_lib_settings"]) _
import com.audi.cortex.jenkins.pipeline.Utils

conan.settings = cortex_jenkins_lib_settings() {
    gitflow = true
    defaultUser = 'fep'
    extraModules = ['atlassian-python-api==3.32.1' ,'lxml==5.1.0'] // required for CoRTEXPythonRequiresHelper ChangelogGenerator
    validateThirdPartyProducts = true
    thirdPartyListName = 'third_party_list_3_3_0.yml'
}

def abi_settings = cortex_jenkins_lib_settings() {
    gitflow = true
    defaultUser = 'fep'
    validateThirdPartyProducts = false
}

def channel = Utils.resolveClosure(conan.settings.channel)

def conanfileDir = "."
def conanfileTests = "conanfile_tests.py"
def conanfileQA = "conanfile_qa.py"
def conanfileAbiDump = "conanfile_abi_dump.py"
def conanfileAbiCheck = "conanfile_abi_check.py"
def conanfileTestCoverage= "conanfile_test_coverage.py"
def conanfileSCA= "conanfile_sca.py"
def conanfileArc = "conanfile_arc.py"

pipeline {
    agent {
        node { label "u18-base" }
    }

    environment {
        // See: https://devstack.vwgroup.com/confluence/display/GXIL/DevHub2+Secrets
        ARTIFACTORY = credentials('DEVSTACK_ARTIFACTORY_USER_API') // for sonar helper
        BITBUCKET = credentials('DEVSTACK_BITBUCKET_USER_API')
        BLACK_DUCK_API_TOKEN = credentials('BLACK_DUCK_API_TOKEN_FEP')
        JIRA = credentials('DEVSTACK_JIRA_USER_API')
        SONAR = credentials('DEVSTACK_SONARQUBE_USER_API')
    }

    triggers {
        cron(env.BRANCH_NAME == 'develop' ? 'H H(2-3) * * 1-5' : '')
    }


    parameters {
        booleanParam(name: "Linux_x86_gcc7_platform",
                    defaultValue: true,
                    description: "Enable Linux_x86_gcc7 platform for build/test stages (depending on the other parameters)")
        booleanParam(name: "Linux_x64_gcc7_platform",
                    defaultValue: true,
                    description: "Enable Linux_x64_gcc7 platform for build/test stages (depending on the other parameters)")
        booleanParam(name: "Linux_armv8_gcc7_platform",
                    defaultValue: true,
                    description: "Enable Linux_armv8_gcc7 platform for build/test stages (depending on the other parameters)")
        booleanParam(name: "Windows_x64_vc142_VS2019_platform",
                    defaultValue: true,
                    description: "Enable Windows_x64_vc142_VS2019 platform for build/test stages (depending on the other parameters)")
        booleanParam(name: "RelWithDebInfo_build_type",
                    defaultValue: true,
                    description: "Enable RelWithDebInfo build type for build/test stages (depending on the other parameters)")
        booleanParam(name: "Debug_build_type",
                    defaultValue: true,
                    description: "Enable Debug build type for build/test stages (depending on the other parameters)")
        booleanParam(name: "Build_stages",
                    defaultValue: true,
                    description: "Enable build stages (platforms depend on the other parameters)")
        booleanParam(name: "Functional_test_stages",
                    defaultValue: true,
                    description: "Enable functional test stages (platforms depend on the other parameters)")
        booleanParam(name: "Black_duck_check",
                     defaultValue: false,
                     description: "Enable Black Duck check during build (available for testing channel only)")
        booleanParam(name: "Static_code_analysis_stage",
                     defaultValue: true,
                     description: "Enable SCA stage including clang-format, gcov test coverage and SonarQube checks")
        booleanParam(name: "Software_architecture_documentation_stage",
                     defaultValue: true,
                     description: "Enable ARC stage for software architecture documentation build")
        booleanParam(name: "QA_stage",
                    defaultValue: true,
                    description: "Enable QA stage (platforms depend on the other parameters)")
        booleanParam(name: "Integration_test_stage",
                    defaultValue: true,
                    description: "Enable integration test stage (platforms depend on the other parameters)")
        booleanParam(name: "Upload_conan_packages",
                    defaultValue: true,
                    description: "Upload Conan packages created by other stages")
    }

    stages {
        stage('Check QA and test package exist') {
            when {
                beforeAgent true
                buildingTag()
            }
            agent {
                docker {
                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                    alwaysPull true
                    args "-u root:root"
                }
            }
            steps {
                conan([clean: false, venv: '']){
                    validateTestPackage channel: 'integration'
                    validateQAPackage channel: 'integration'
                }
            }
        }
        stage('Platform Lanes') {
            parallel {
                stage('Linux_x86_gcc7 RelWithDebInfo') {
                    when {
                        expression {
                            params.Linux_x86_gcc7_platform && params.RelWithDebInfo_build_type
                        }
                    }
                    stages {
                        stage('Build RelWithDebInfo') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-i386-pasit:0.3.1'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileDir,
                                                 createArgs: [ "-pr Linux_x86_gcc7 -e ENABLE_DOCUMENTATION=False -s rti_connext_dds:compiler.version=5 -s rti_connext_dds:arch_build=x86  -s rti_connext_dds:os_build=Linux" ],
                                                 failOnMissingVenv: false,
                                                 ignoreDirty: true,
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults()
                                    cleanWs()
                                }
                            }
                        }
                        stage('Test RelWithDebInfo') {
                            when {
                                expression {
                                    params.Functional_test_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-i386-pasit:0.3.1'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileTests,
                                                 createArgs: [ "-pr Linux_x86_gcc7" ],
                                                 failOnMissingVenv: false,
                                                 ignoreDirty: true,
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                    }
                }
                stage('Linux_x64_gcc7 RelWithDebInfo') {
                    when {
                        expression {
                            params.Linux_x64_gcc7_platform && params.RelWithDebInfo_build_type
                        }
                    }
                    stages {
                        stage('Build RelWithDebInfo') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                                RUN_BLACK_DUCK_CHECK = "${params.Black_duck_check==false?'False':'True'}"
                                JENKINS_NODE_TO_RUN_BLACK_DUCK_CHECK = "True"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileDir,
                                                 createArgs: ['-pr Linux_x64_gcc7 -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                        stage('Test RelWithDebInfo') {
                            when {
                                expression {
                                    params.Functional_test_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                                RUN_BLACK_DUCK_CHECK = "${params.Black_duck_check==false?'False':'True'}"
                                JENKINS_NODE_TO_RUN_BLACK_DUCK_CHECK = "True"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileTests, 
                                                 createArgs: ['-pr Linux_x64_gcc7 -o fep_sdk_participant:shared=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                    }
                }
                stage('Linux_x64_gcc7 Debug') {
                    when {
                        expression {
                            params.Linux_x64_gcc7_platform && params.Debug_build_type
                        }
                    }
                    stages {
                        stage('Build Debug') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileDir,
                                                 createArgs: ['-pr Linux_x64_gcc7 -s build_type=Debug -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                        stage('Test Debug') {
                            when {
                                expression {
                                    params.Functional_test_stages
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileTests,
                                                 createArgs: ['-pr Linux_x64_gcc7 -s build_type=Debug -o fep_sdk_participant:shared=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                        stage('Abi Dump release & beta') {
                            when {
                                anyOf {
                                    expression { env.BRANCH_NAME.contains("release") }
                                    expression {
                                        if (env.TAG_NAME) {
                                            return env.TAG_NAME.contains("beta") ||
                                                   env.TAG_NAME.contains("release") ||
                                                   env.TAG_NAME ==~ /v[0-9]+\.[0-9]+\.[0-9](.*)/
                                        } else { return false }
                                    }
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '', settings: abi_settings]) {
                                    checkout scm
                                    createUpload(conanFile: conanfileAbiDump,
                                                 createArgs: ['-pr Linux_x64_gcc7 -s build_type=Debug'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml"])
                                    cleanWs()
                                }
                            }
                        }
                        stage('Abi compliance check'){
                            // disable check for integration or release until FEPSDK-3697 is fixed
                            when {
                                not{
                                    anyOf {
                                        expression { env.BRANCH_NAME.contains("release") }
                                        expression {
                                            if (env.TAG_NAME) {
                                                return env.TAG_NAME.contains("beta") ||
                                                       env.TAG_NAME.contains("release") ||
                                                       env.TAG_NAME ==~ /v[0-9]+\.[0-9]+\.[0-9](.*)/
                                            } else { return false }
                                        }
                                    }
                                }
                            }
                            agent {
                                docker {
                                    image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                                    alwaysPull true
                                    args "-u root:root"
                                }
                            }
                            environment {
                                FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '', settings: abi_settings]) {
                                    checkout scm
                                    createUpload(conanFile: conanfileAbiCheck,
                                                 createArgs: ['-pr Linux_x64_gcc7  -s build_type=Debug'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml"])
                                    cleanWs()
                                }
                            }
                        }
                    }
                }
                stage('Linux_armv8_gcc7 RelWithDebInfo') {
                    when {
                        expression { params.Linux_armv8_gcc7_platform && params.RelWithDebInfo_build_type }
                    }
                    agent {
                        docker {
                            label 'Linux_ARM_Docker'
                            image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u20-arm-pasit:0.4.0'
                            alwaysPull true
                            args "-u root:root"
                        }
                    }
                    environment {
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                    }
                    stages {
                        stage('Build RelWithDebInfo') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileDir,
                                                 createArgs: ['-pr Linux_armv8_gcc7 -s build_type=RelWithDebInfo -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                }
                            }
                        }
                        stage('Test RelWithDebInfo') {
                            when {
                                expression {
                                    params.Functional_test_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileTests,
                                                 createArgs: ['-pr Linux_armv8_gcc7 -s build_type=RelWithDebInfo -o fep_sdk_participant:shared=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                }
                            }
                        }
                    }
                    post {
                        always {
                            cleanWs()
                        }
                    }
                }
                stage('Linux_armv8_gcc7 Debug') {
                    when {
                        expression {
                            params.Linux_armv8_gcc7_platform && params.Debug_build_type
                        }
                    }
                    agent {
                        docker {
                            label 'Linux_ARM_Docker'
                            image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u20-arm-pasit:0.4.0'
                            alwaysPull true
                            args "-u root:root"
                        }
                    }
                    environment {
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                    }
                    stages {
                        stage('Build Debug') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']) {
                                    checkout scm
                                    createUpload(conanFile: conanfileDir,
                                                 createArgs: ['-pr Linux_armv8_gcc7 -s build_type=Debug -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    cleanWs()
                                }
                            }
                        }
                    }
                }
                stage('Windows_x64_vc142_VS2019 RelWithDebInfo') {
                    when {
                        expression {
                            params.Windows_x64_vc142_VS2019_platform && params.RelWithDebInfo_build_type
                        }
                    }
                    agent { label "Windows_x64_vc142_VS2019" }
                    environment {
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                    }
                    stages {
                        stage('Build RelWithDebInfo') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']){
                                    checkout scm
                                    createUpload(conanFile: conanfileDir, 
                                                 createArgs: ['-pr Windows_x64_vc142_VS2019 -s build_type=RelWithDebInfo -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)    
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                }
                            }
                        }
                        stage('Test RelWithDebInfo') {
                            when {
                                expression {
                                    params.Functional_test_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '']){
                                    checkout scm
                                    createUpload(conanFile: conanfileTests, 
                                                 createArgs: ['-pr Windows_x64_vc142_VS2019 -s build_type=RelWithDebInfo -o fep_sdk_participant:shared=True'],
                                                 upload: params.Upload_conan_packages)    
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                }
                            }
                        }
                    }
                    post {
                        always {
                            cleanWs()
                        }
                    }
                }
                stage('Windows_x64_vc142_VS2019 Debug') {
                    when {
                        expression {
                            params.Windows_x64_vc142_VS2019_platform && params.Debug_build_type
                        }
                    }
                    agent { label "Windows_x64_vc142_VS2019" }
                    environment {
                        OPENCPPCOVERAGE_OUTPUT = "${pwd()}/test/results/opencppcoverage"
                        RUN_CODE_COVERAGE_CHECK = "${env.BRANCH_NAME == "develop" ? "True" : "False"}"
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                    }
                    stages {
                        stage('Build Debug') {
                            when {
                                expression {
                                    params.Build_stages
                                }
                            }
                            steps {
                                conan([clean: false, checkout: false, venv: '', settings: conan.settings]){
                                    checkout scm
                                    createUpload(conanFile: conanfileDir, 
                                                 createArgs: ['-pr Windows_x64_vc142_VS2019 -s build_type=Debug -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                                 upload: params.Upload_conan_packages)
                                    script {
                                        if (env.BRANCH_NAME == 'develop') {
                                                createUpload( conanFile: conanfileTestCoverage,
                                                createArgs: ['-pr Windows_x64_vc142_VS2019 -s build_type=Debug'],
                                                upload: params.Upload_conan_packages)
                                        }
                                    }
                                }
                            }
                            post {
                                always {
                                    conanPublishTestResults(["**/test*.xml", "**/result_install_complete.xml"])
                                    script {
                                        if (env.BRANCH_NAME == 'develop') {
                                            publishHTML([allowMissing: false,
                                                         alwaysLinkToLastBuild: false,
                                                         keepAll: false,
                                                         reportDir: 'test/results/opencppcoverage',
                                                         reportFiles: 'index.html',
                                                         reportName: 'OpenCPP Coverage Report',
                                                         reportTitles: ''])
                                        }
                                    }
                                }
                            }
                        }
                    }
                    post {
                        always {
                            cleanWs()
                        }
                    }
                }
                stage('SCA Linux_x64_gcc7') {
                    when {
                        expression {
                            params.Static_code_analysis_stage
                        }
                    }
                    agent {
                        docker {
                            image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.4.0'
                            alwaysPull true
                            args "-u root:root"
                        }
                    }
                    environment {
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                        RUN_CODE_COVERAGE_CHECK = "True"
                        ENABLE_FILE_FORMATTING = "True"
                    }
                    steps {
                        conan([clean: false, checkout: false, venv: '']) {
                            checkout scm
                            createUpload(conanFile: conanfileSCA,
                                         createArgs: ['-pr Linux_x64_gcc7 -s build_type=RelWithDebInfo -o fep_sdk_participant:shared=True -o fep_sdk_participant:use_rtidds=True'],
                                         upload: params.Upload_conan_packages)
                        }
                    }
                    post {
                        always {
                            cleanWs()
                        }
                    }
                }
                stage('ARC Windows_x64_vc142_VS2019') {
                    when {
                        expression {
                            params.Software_architecture_documentation_stage
                        }
                    }
                    agent { label "Windows_x64_vc142_VS2019" }
                    environment {
                        FEP_SETTINGS_FOLDER = "${env.WORKSPACE}/.fep"
                    }
                    steps {
                        conan([clean: false, checkout: false, venv: '', extraModules: ['sphinx==5.0.1', 'sphinx_rtd_theme==1.0.0', 'breathe==4.34.0', 'sphinxmark==1.0.0', 'sphinxcontrib-spelling==8.0.0']]) {
                            checkout scm
                            createUpload(conanFile: conanfileArc,
                                         createArgs: ['-pr Windows_x64_vc142_VS2019 -s build_type=RelWithDebInfo'],
                                         upload: params.Upload_conan_packages)
                        }
                    }
                    post {
                        always {
                            cleanWs()
                        }
                    }
                }
            }
        }
        stage('QA and Integration Tests') {
            parallel {
                stage('QA for Release Builds and Tests') {
                    when {
                        expression {
                            params.QA_stage
                        }
                    }
                    agent { label "QA" }
                    steps {
                        conanCreateUpload(conanFile: conanfileQA, 
                                          upload: params.Upload_conan_packages)
                    }
                    post {
                        always {
                            conanPublishTestResults(["**/test*.xml"])
                            cleanWs()
                        }
                    }
                }
                stage ('FEP Integration Tests') {
                    when {
                        changeRequest target: 'develop'
                        expression {
                            params.Integration_test_stage
                        }
                    }
                    agent {
                        docker {
                            image 'fepdev-docker-internal.docker.devstack.vwgroup.com/u18-pasit:0.3.1'
                            alwaysPull true
                            args "-u root:root"
                        }
                    }
                    steps {
                        conan([clean: false, checkout: false, venv: '']) {
                            checkout scm
                            conanInspect() { inspect ->
                                build job: 'PASIT/FEP/fep_integration/simulation_framework_integration_tests/main', parameters: [
                                    [$class: 'StringParameterValue', name: "FEP_SDK_PARTICIPANT_VERSION", value: inspect.version],
                                    [$class: 'StringParameterValue', name: "FEP_SDK_PARTICIPANT_CHANNEL", value: channel]]
                            }
                        }
                    }
                }
            }
        }
    }
}
