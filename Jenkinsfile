libraries {
     lib('pipeline-library-demo')
}

pipeline {
    agent { docker { image 'python:3.5.1' } }
    environment {
        DISABLE_AUTH = 'true'
        DB_ENGINE    = 'sqlite'
    }
    stages {
        stage('HELLO WORLD') {
            steps {
                sh 'python --version'
            }
        }

	stage('RUN LOCAL SCRIPT') {
		steps {
			sh 'bash ./test_bash_script.sh'
			echo "Database engine is ${DB_ENGINE}"
		}
	}

	stage('Demo Step of Jenkins pipeline') {
		steps {
			echo 'Hello world'
			sayHello 'Dave'
		}
 	}

    }

    post {
        always {
            echo 'This will always run'
        }
        success {
            echo 'This will run only if successful'
        }
        failure {
            echo 'This will run only if failed'
        }
        unstable {
            echo 'This will run only if the run was marked as unstable'
        }
        changed {
            echo 'This will run only if the state of the Pipeline has changed'
            echo 'For example, if the Pipeline was previously failing but is now successful'
        }
    }
}
